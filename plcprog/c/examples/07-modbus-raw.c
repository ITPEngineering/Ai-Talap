#include <stdint.h>
#include <stdbool.h>
#include <logicbox.h>

#define PORT (1)                /* using modbus_client1 */
#define SLAVE1 (1)
#define RESVAR "res3"           /* every prog should have its own resvar */

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


void READ_RAW(ModbusRequest *fb){
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

    if (rising_edge && !fb->xBusy) {
        fb->xBusy = true;
        fb->xDone = false;
        fb->xError = false;
        fb->eError = 0;
        fb->_stamp = lbtime();

        // 1. Формируем Modbus RTU запрос (8 байт для функции 0x03)
        uint8_t tx_buf[8];
        tx_buf[0] = fb->uiSlaveId;
        tx_buf[1] = fb->fctCode;
        tx_buf[2] = (uint8_t)(fb->uiAddress >> 8);
        tx_buf[3] = (uint8_t)(fb->uiAddress & 0xFF);
        tx_buf[4] = (uint8_t)(fb->uiQuantity >> 8);
        tx_buf[5] = (uint8_t)(fb->uiQuantity & 0xFF);

        uint16_t crc = lbmodbus_crc(tx_buf, 6);
        tx_buf[6] = (uint8_t)(crc & 0xFF);        // Младший байт CRC
        tx_buf[7] = (uint8_t)(crc >> 8);          // Старший байт CRC

        // 2. Рассчитываем ожидаемую длину ответа (rlen)
        // Для функции 0x03: Слейв_ID(1) + Функ(1) + Кол-во_байт(1) + Данные(N*2) + CRC(2) = 5 + N*2
        int expected_rlen = 5 + (fb->uiQuantity * 2);

        // Отправляем через lb485
        int err = lb485(PORT, tx_buf, sizeof(tx_buf), expected_rlen, RESVAR, fb->timeout);
        if (err) {
            lblog("lb485 err=%d", err);
            fb->xBusy = false;
            fb->xError = true;
            fb->eError = err;
        }

        char str[sizeof(tx_buf)*3+1];
        for (int i = 0; i < sizeof(tx_buf); i++) {
            snprintf (&str[i*3], 4, " %02x", tx_buf[i]);
        }
        lblog("Sent tx_buf:%s", str);
        return;
    }


    if (fb->xBusy && fb->xExecute) {
        if (lbint64("stamp."RESVAR) <= fb->_stamp) {
            return; // Данные еще не пришли
        }

        // Буфер для приема сырых байт
        int expected_rlen = 5 + (fb->uiQuantity * 2);
        uint8_t rx_buf[256];

        int count = lbbin(RESVAR, rx_buf, sizeof(rx_buf));
        if (count >= 2 && *(int16_t *)rx_buf) {
            lblog("Operation error code %d", *(int16_t *)rx_buf);
            fb->xError = true;
            fb->eError = *(int16_t *)rx_buf;
            fb->xBusy = false;
            return;
        }
        if (count < expected_rlen) {
            lblog("Error: received bytes count %d less than expected %d", count, expected_rlen);
            fb->xError = true;
            fb->eError = -1001; // Ошибка длины
            fb->xBusy = false;
            return;
        }

        // Проверка CRC-16 принятого пакета
        uint16_t calc_crc = lbmodbus_crc(&rx_buf[2], count - 4);
        uint16_t rx_crc = (uint16_t)rx_buf[count - 1] << 8 | rx_buf[count - 2];
        if (calc_crc != rx_crc) {
            char str[count*3+1];
            for (int i = 0; i < count; i++) {
                snprintf (&str[i*3], 4, " %02x", rx_buf[i]);
            }
            lblog("Error: CRC mismatch. Calc: 0x%04X, Rx: 0x%04X rx_buf:%s", calc_crc, rx_crc, str);
            fb->xError = true;
            fb->eError = -1003; // Ошибка CRC
            fb->xBusy = false;
            return;
        }

        // Проверка Modbus Exception (ошибки от устройства)
        // Если старший бит функции равен 1 (например, 0x83 вместо 0x03)
        if (rx_buf[3] == (fb->fctCode | 0x80)) {
            uint8_t exception_code = rx_buf[2];
            lblog("Modbus Exception code: %d", exception_code);
            fb->xError = true;
            fb->eError = exception_code? -exception_code: LOGICBOX_ERR_EXCEPTION0;
            fb->xBusy = false;
            return;
        }

        // Проверка размера пользовательского буфера назначения
        if ((fb->uiQuantity * sizeof(uint16_t)) > fb->szBufferSize) {
            fb->xError = true;
            fb->eError = -1003;
            fb->xBusy = false;
            return;
        }

        uint16_t *dst_buf = (uint16_t *)fb->pBuffer;

        int data_start_index = 5;

        for (int i = 0; i < fb->uiQuantity; i++) {
            uint8_t hi = rx_buf[data_start_index + (i * 2)];
            uint8_t lo = rx_buf[data_start_index + (i * 2) + 1];
            dst_buf[i] = ((uint16_t)hi << 8) | lo;
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
    fbRead.xExecute = lbvar("read_raw");
    READ_RAW(&fbRead);
    if (fbRead.xDone && fbRead.xExecute) {
        lblog("Цикл завершен! Данные обновлены: 0x%04X, 0x%04X, 0x%04X, 0x%04X, 0x%04X, 0x%04X, 0x%04X, 0x%04X, 0x%04X, 0x%04X",
              read_reg[0], read_reg[1], read_reg[2], read_reg[3], read_reg[4],
              read_reg[5], read_reg[6], read_reg[7], read_reg[8], read_reg[9]);
    }

    if (fbRead.xError&& fbRead.xExecute) {
        lblog("Ошибка опроса: %d. Пробуем снова...", fbRead.eError);
    }
}
