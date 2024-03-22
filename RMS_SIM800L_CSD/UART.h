hw_timer_t * timer = NULL;
#define NUMERO_PORTA_SERIALE UART_NUM_1
#define NUMERO_PORTB_SERIALE UART_NUM_2
#define BUF_SIZE (1024 * 2)
#define RD_BUF_SIZE (1024)
static QueueHandle_t uart1_queue;
static QueueHandle_t uart2_queue;

static const char * TAG = "";

uint8_t rxbuf[256];     //Buffer di ricezione
uint16_t rx_fifo_len;        //Lunghezza dati

uart_event_t event;
size_t buffered_size;
bool exit_condition = false;