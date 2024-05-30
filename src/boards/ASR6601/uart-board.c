#include "utilities.h"
#include "board.h"
#include "tremo_uart.h"
#include "tremo_rcc.h"
#include "uart-board.h"

#define TX_BUFFER_RETRY_COUNT 10

uint8_t RxData = 0;
uint8_t TxData = 0;

extern Uart_t Uart0;

void UartMcuInit(Uart_t *obj, UartId_t uartId, PinNames tx, PinNames rx) {
    obj->UartId = uartId;
    GpioInit( &obj->Tx, tx, PIN_ALTERNATE_FCT, PIN_PUSH_PULL, PIN_PULL_UP, 1);
    GpioInit( &obj->Rx, rx, PIN_ALTERNATE_FCT, PIN_PUSH_PULL, PIN_PULL_UP, 1);
}

uart_t* get_uart_address(UartId_t uart){
    switch(uart) {
        case UART_USB_CDC:
            return UART0;
         case UART_1:
            return UART1;
        case UART_2:
            return UART2;
     }
} 

uint32_t get_rcc_uart_address(UartId_t uart){
    switch(uart) {
        case UART_USB_CDC:
            return RCC_PERIPHERAL_UART0;
        case UART_1:
            return RCC_PERIPHERAL_UART1;
        case UART_2:
            return RCC_PERIPHERAL_UART2;
    }
} 

void UartMcuConfig(Uart_t *obj, UartMode_t mode, uint32_t baudrate, WordLength_t wordLength, 
    StopBits_t stopBits, Parity_t parity, FlowCtrl_t flowCtrl) {
    uart_config_t uart_config;
    uart_t* uart = get_uart_address(obj->UartId);
    uart_config_init(&uart_config);

    uart_config.baudrate = baudrate;

    switch(wordLength) {
        case UART_8_BIT: 
            uart_config.data_width = UART_DATA_WIDTH_8;
            break;
        default:
            break;
    }
    switch(stopBits) {
        case UART_1_STOP_BIT:
            uart_config.stop_bits = UART_STOP_BITS_1;
            break;
        case UART_2_STOP_BIT:
            uart_config.stop_bits = UART_STOP_BITS_2;
            break;
        default:
            break;
    }
    switch(parity) {
        case NO_PARITY:
            uart_config.parity = UART_PARITY_NO;
            break;
        case EVEN_PARITY:
            uart_config.parity = UART_PARITY_EVEN;
            break;
        case ODD_PARITY:
            uart_config.parity = UART_PARITY_ODD;
            break;
        default:
            break;
    }

    switch(flowCtrl) {
        case NO_FLOW_CTRL:
            uart_config.flow_control = UART_FLOW_CONTROL_DISABLED;
            break;
        case RTS_FLOW_CTRL:
            uart_config.flow_control = UART_FLOW_CONTROL_RTS;
            break;
        case CTS_FLOW_CTRL:
            uart_config.flow_control = UART_FLOW_CONTROL_CTS;
            break;
        case RTS_CTS_FLOW_CTRL:
            uart_config.flow_control = UART_FLOW_CONTROL_CTS_RTS;
            break;
        default:
            break;
    }
        
    rcc_enable_peripheral_clk(get_rcc_uart_address(obj->UartId), true);
    uart_init(uart, &uart_config);
    uart_cmd(uart, ENABLE);
}

void UartMcuDeInit(Uart_t *obj ){
    uart_deinit(get_uart_address(obj->UartId));
    obj->IsInitialized = false;
    GpioInit( &obj->Tx, obj->Tx.pin, PIN_ANALOGIC, PIN_PUSH_PULL, PIN_NO_PULL, 0);
    GpioInit( &obj->Rx, obj->Rx.pin, PIN_ANALOGIC, PIN_PUSH_PULL, PIN_NO_PULL, 0);
}

uint8_t UartMcuPutChar( Uart_t *obj, uint8_t data ) {
    if(get_uart_address(obj->UartId)->FR & UART_FLAG_BUSY) 
        return 1;
    CRITICAL_SECTION_BEGIN();
    uart_send_data(get_uart_address(obj->UartId), data);
    CRITICAL_SECTION_END();
    return 0;
}

uint8_t UartMcuGetChar( Uart_t *obj, uint8_t *data ) {
    uint8_t ret;
    if(get_uart_address(obj->UartId)->FR & UART_FLAG_BUSY) 
        return 1;
    CRITICAL_SECTION_BEGIN();
    ret = uart_recieve_data(get_uart_address(obj->UartId));
    CRITICAL_SECTION_END();
    *data = ret;
    return 0;
}

uint8_t UartMcuPutBuffer(Uart_t *obj, uint8_t *buffer, uint16_t size) {
    uint16_t i;
    for(i = 0; i < size; i++) {
        if(UartMcuPutChar(obj, *(buffer + i)))
            return 1;
    }
}

uint8_t UartMcuGetBuffer(Uart_t *obj, uint8_t *buffer, uint16_t size, uint16_t *nbReadBytes){
    uint16_t i;
    uart_t* uart = get_uart_address(obj->UartId);
    for(i = 0; i < size; i++) {
        if(UartMcuGetChar(obj, buffer + i))
            return 1;
        if(!(uart->RSC_ECR))
            *(nbReadBytes) += 1;
    }
}
