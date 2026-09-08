#include <stdint.h>
#include <stdbool.h>
#include <logicbox.h>

#define PORT (1)                /* using modbus_client1 */
#define SLAVE1 (1)
#define RESVAR "res2"           /* every prog should have its own resvar */

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


static ModbusRequest fbWrite;
static uint16_t write_reg[10] = {0};

void WRITE_VAR(ModbusRequest *fb){
    bool rising_edge = fb->xExecute && !fb->_oldExecute;
    if ((fb->xDone || fb->xError) && fb->xExecute) {
        fb->_oldExecute = false;
        fb->xDone = false;
        fb->xError = false;
        rising_edge = true;
    } else {
        fb->_oldExecute = fb->xExecute;
    }

    if (rising_edge && !fb->xBusy) {
        // Проверка: выделил ли пользователь буфер нужного размера для отправки данных?
        if (fb->pWriteBuffer == NULL || fb->szWriteBufferSize < (fb->uiQuantity * sizeof(uint16_t))) {
            fb->xError = true;
            fb->eError = -1002; // Ошибка: неверный или маленький буфер записи
            return;
        }

        fb->xBusy = true;
        fb->xDone = false;
        fb->xError = false;
        fb->eError = 0;
        fb->_stamp = lbtime();

        uint16_t *buf = (uint16_t *)fb->pWriteBuffer;
        lblog("Состояние буфера до вызова lbmodbus: 0x%04X, 0x%04X, 0x%04X, 0x%04X, 0x%04X, 0x%04X, 0x%04X, 0x%04X, 0x%04X, 0x%04X",
              buf[0], buf[1], buf[2], buf[3], buf[4], buf[5], buf[6], buf[7], buf[8], buf[9]);
        // Вызываем lbmodbus, передавая подготовленный буфер отправки
        int err = lbmodbus(PORT, fb->uiSlaveId, fb->fctCode, fb->uiAddress, fb->uiQuantity, buf, RESVAR, fb->timeout);
        if (err) {
            lblog ("lbmodbus write err=%d", err);
            fb->xBusy = false;
            fb->xError = true;
            fb->eError = err;
        }
        return;
    }

    if (fb->xBusy) {
        if (lbint64("stamp."RESVAR) <= fb->_stamp) {
            lblog("operation WRITE_BUSY... System Stamp: %lld, Block Stamp: %lld",
                  lbint64("stamp."RESVAR),
                  fb->_stamp);
            return;
        }

        // При записи драйвер Logic Box возвращает в RESVAR только статус выполнения (ошибку)
        int16_t buf[1] = {0};
        int count = lbbin(RESVAR, buf, sizeof(buf));

        if (count < 2 || buf[0] != 0) {
            int16_t errn = (count >= 2) ? buf[0] : -1001;
            lblog ("write operation error code %d", errn);
            fb->xError = true;
            fb->eError = errn;
            fb->xBusy = false;
            return;
        }

        // Операция записи успешно завершена
        fb->xDone = true;
        fb->xBusy = false;
    }
}

void init()
{
    fbWrite.uiSlaveId = SLAVE1;
    fbWrite.fctCode = 0x10; // 0x10 - Write Multiple Registers
    fbWrite.uiAddress = 10;
    fbWrite.uiQuantity = 10;
    fbWrite.pWriteBuffer = &write_reg;
    fbWrite.szWriteBufferSize = sizeof(write_reg);
    fbWrite.timeout = (int)1000e3;
    fbWrite.xExecute = false;
}


void run()
{
    fbWrite.xExecute = lbvar("wrt");
    WRITE_VAR(&fbWrite);
    if (fbWrite.xDone && fbWrite.xExecute) {
        lblog("Запись 10 регистров на устройство %d успешно завершена!", fbWrite.uiSlaveId);
        for (int i = 0; i < fbWrite.uiQuantity; i++) {
            write_reg[i]++;
        }
    }

    if (fbWrite.xError&& fbWrite.xExecute) {
        lblog("Ошибка записи: %d", fbWrite.eError);
    }
}
