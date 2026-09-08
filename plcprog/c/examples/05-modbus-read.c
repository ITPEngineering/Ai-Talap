#include <stdint.h>
#include <stdbool.h>
#include <logicbox.h>

#define PORT (1)                /* using modbus_client1 */
#define SLAVE1 (1)
#define RESVAR "res1"           /* every prog should have its own resvar */

typedef struct {
    // IN
    bool xExecute;
    uint8_t uiSlaveId;
    uint8_t fctCode;
    uint16_t uiAddress;
    uint16_t uiQuantity;
    void *pBuffer;
    uint32_t szBufferSize;
    void *pWriteBuffer;    // Для записи: откуда берем данные
    uint32_t szWriteBufferSize; // Размер буфера записи
    int timeout;

    // OUT
    bool xDone;
    bool xBusy;
    bool xError;
    int32_t eError;

    // EXT
    bool _oldExecute;
    int64_t _stamp;
} ModbusRequest;

static ModbusRequest fbRead;
static uint16_t read_reg[10] = {0};

void READ_VAR(ModbusRequest *fb){
    // Детектор фронта
    bool rising_edge = fb->xExecute && !fb->_oldExecute;

    // Если операция УЖЕ завершилась (есть Done или Error),
    // но пользователь держит xExecute постоянно в true
    if ((fb->xDone || fb->xError) && fb->xExecute) {
        fb->_oldExecute = false;
        fb->xDone = false;
        fb->xError = false;
        rising_edge = true;
    } else {
        fb->_oldExecute = fb->xExecute;
    }

    // Стартуем по фронту, если не заняты
    if (rising_edge && !fb->xBusy) {
        fb->xBusy = true;
        fb->xDone = false;
        fb->xError = false;
        fb->eError = 0;
        fb->_stamp = lbtime();

        int err = lbmodbus(PORT, fb->uiSlaveId, fb->fctCode, fb->uiAddress, fb->uiQuantity, NULL, RESVAR, fb->timeout);
        if (err) {
            lblog ("lbmodbus err=%d", err);
            fb->xBusy = false;
            fb->xError = true;
            fb->eError = err;
        }
        return;
    }

    // Проверка асинхронного ответа в последующих циклах run()
    if (fb->xBusy && fb->xExecute) {
        if (lbint64("stamp."RESVAR) <= fb->_stamp) {
            lblog("operation READ_BUSY... System Stamp: %lld, Block Stamp: %lld",
                  lbint64("stamp."RESVAR),
                  fb->_stamp);
            return; // Данные еще не пришли от RS-485. Выходим и ждем следующий цикл run()
        }

        uint16_t buf[1 + fb->uiQuantity];
        int count = lbbin(RESVAR, buf, sizeof(buf));
        lblog ("count= %d, buf[0]= %d, buf[1]= %d", count, buf[0], buf[1]);

        if (count < 2 || buf[0] != 0) {
            int16_t errn = (count >= 2) ? buf[0] : -1001;
            lblog ("operation error code %d", errn);
            fb->xError = true;
            fb->eError = errn;
            fb->xBusy = false;
            return;
        }

        uint16_t *dst_buf = (uint16_t *)fb->pBuffer;
        if ((fb->uiQuantity * sizeof(uint16_t)) <= fb->szBufferSize) {
            for (int i = 0; i < fb->uiQuantity; i++) {
                dst_buf[i] = buf[i+1];
            }
        } else {
            fb->xError = true;
            fb->eError = -1002;
            fb->xBusy = false;
            return;
        }
        fb->xDone = true;
        fb->xBusy = false;
    }
}

void init()
{
    fbRead.uiSlaveId = SLAVE1;
    fbRead.fctCode = 0x03; // Read Holding Registers
    fbRead.uiAddress = 0;
    fbRead.uiQuantity = 10;
    fbRead.pBuffer = &read_reg;
    fbRead.szBufferSize = sizeof(read_reg);
    fbRead.timeout = (int)1000e3;
    fbRead.xExecute = true;
}


void run()
{
    fbRead.xExecute = lbvar("read");
    READ_VAR(&fbRead);
    if (fbRead.xDone && fbRead.xExecute) {
        lblog("Цикл завершен! Данные обновлены: 0x%04X, 0x%04X, 0x%04X, 0x%04X, 0x%04X, 0x%04X, 0x%04X, 0x%04X, 0x%04X, 0x%04X",
              read_reg[0], read_reg[1], read_reg[2], read_reg[3], read_reg[4],
              read_reg[5], read_reg[6], read_reg[7], read_reg[8], read_reg[9]);
    }

    if (fbRead.xError && fbRead.xExecute) {
        lblog("Ошибка опроса: %d. Пробуем снова...", fbRead.eError);
    }
}
