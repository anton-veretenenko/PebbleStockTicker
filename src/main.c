#include "pebble.h"

void handle_minute_tick(struct tm *tick_time, TimeUnits units_changed);
void fetch_price();
static void app_message_init(void);
	
Window *window;
TextLayer *text_date_layer;
TextLayer *text_time_layer;
TextLayer *text_day_layer;
TextLayer *text_ticker_layer;
TextLayer *text_ticker_london_layer;
static bool show_dots = true;
static bool first_run = true;

static char ticker_text[] = "URKA  000.00 D";
static char ticker_london_text[] = "LOND    00.00 D";
enum {
	KEY_FETCH = 0x0,
	KEY_PRICE = 0x1,
	KEY_PRICE_LONDON = 0x2,
};

void handle_second_tick(struct tm *tick_time, TimeUnits units_changed)
{
	static char time_text[] = "03.44";
	char *time_format;
	if (show_dots) {
		if (clock_is_24h_style()) {
			time_format = "%H.%M";
		} else {
			time_format = "%I.%M";
		}
	} else {
		if (clock_is_24h_style()) {
			time_format = "%H %M";
		} else {
			time_format = "%I %M";
		}
	}
	//show_dots ^= 1;
	
	strftime(time_text, sizeof(time_text), time_format, tick_time);

	// Kludge to handle lack of non-padded hour format string
	// for twelve hour clock.
	if (!clock_is_24h_style() && (time_text[0] == '0')) {
		memmove(time_text, &time_text[1], sizeof(time_text) - 1);
	}
	text_layer_set_text(text_time_layer, time_text);
	
	if (first_run) {
		//fetch_price();
	}
	if (units_changed & HOUR_UNIT || first_run) {
		// call js app to fetch price
		handle_minute_tick(tick_time, units_changed);
		first_run = false;
	}
	if (!first_run && units_changed & HOUR_UNIT) {
		fetch_price();
	}
}

void fetch_price()
{
  Tuplet fetch_tuple = TupletInteger(KEY_FETCH, 1);

  DictionaryIterator *iter;
  app_message_outbox_begin(&iter);

  if (iter == NULL) {
    return;
  }

  dict_write_tuplet(iter, &fetch_tuple);
  dict_write_end(iter);

  app_message_outbox_send();	
}

void handle_minute_tick(struct tm *tick_time, TimeUnits units_changed)
{
  static char date_text[] = "SEPTEMBER 29";
	static char day_text[] = "Wednesday";

  strftime(date_text, sizeof(date_text), "%B %e", tick_time);
  text_layer_set_text(text_date_layer, date_text);
	
	strftime(day_text, sizeof(day_text), "%A", tick_time);
	text_layer_set_text(text_day_layer, day_text);
}

void handle_deinit(void) {
  tick_timer_service_unsubscribe();
	app_message_deregister_callbacks();
	text_layer_destroy(text_date_layer);
	text_layer_destroy(text_time_layer);
	text_layer_destroy(text_day_layer);
	text_layer_destroy(text_ticker_layer);
	text_layer_destroy(text_ticker_london_layer);
	window_destroy(window);
}

void handle_init(void) {
  window = window_create();
	app_message_init();
  window_stack_push(window, true /* Animated */);
  window_set_background_color(window, GColorWhite);

  Layer *window_layer = window_get_root_layer(window);
	
	text_ticker_layer = text_layer_create(GRect(0, 0, 144, 26));
	text_layer_set_text_color(text_ticker_layer, GColorWhite);
	text_layer_set_background_color(text_ticker_layer, GColorBlack);
	text_layer_set_font(text_ticker_layer, fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_JERSEY_20)));
	text_layer_set_text_alignment(text_ticker_layer, GTextAlignmentCenter);
	layer_add_child(window_layer, text_layer_get_layer(text_ticker_layer));
	
	text_ticker_london_layer = text_layer_create(GRect(0, 26, 144, 26));
	text_layer_set_text_color(text_ticker_london_layer, GColorWhite);
	text_layer_set_background_color(text_ticker_london_layer, GColorBlack);
	text_layer_set_font(text_ticker_london_layer, fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_JERSEY_20)));
	text_layer_set_text_alignment(text_ticker_london_layer, GTextAlignmentCenter);
	layer_add_child(window_layer, text_layer_get_layer(text_ticker_london_layer));
	
	text_day_layer = text_layer_create(GRect(3, 61, 144-6, 26));
	text_layer_set_text_color(text_day_layer, GColorBlack);
	text_layer_set_background_color(text_day_layer, GColorClear);
	text_layer_set_font(text_day_layer, fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_JERSEY_20)));
	layer_add_child(window_layer, text_layer_get_layer(text_day_layer));
	
  text_date_layer = text_layer_create(GRect(0, 145, 144-2, 26));
  text_layer_set_text_color(text_date_layer, GColorBlack);
  text_layer_set_background_color(text_date_layer, GColorClear);
  text_layer_set_font(text_date_layer, fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_JERSEY_20)));
	text_layer_set_text_alignment(text_date_layer, GTextAlignmentRight);
  layer_add_child(window_layer, text_layer_get_layer(text_date_layer));

  text_time_layer = text_layer_create(GRect(0, 80, 144, 70));
  text_layer_set_text_color(text_time_layer, GColorBlack);
  text_layer_set_background_color(text_time_layer, GColorClear);
  text_layer_set_font(text_time_layer, fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_JERSEY_56)));
	text_layer_set_text_alignment(text_time_layer, GTextAlignmentCenter);
  layer_add_child(window_layer, text_layer_get_layer(text_time_layer));

  //tick_timer_service_subscribe(MINUTE_UNIT, handle_minute_tick);
	tick_timer_service_subscribe(SECOND_UNIT, handle_second_tick);
  // TODO: Update display here to avoid blank display on launch?
}

static void in_received_handler(DictionaryIterator *iter, void *context)
{
  Tuple *price_tuple = dict_find(iter, KEY_PRICE);
	Tuple *price_london_tuple = dict_find(iter, KEY_PRICE_LONDON);
	
	APP_LOG(APP_LOG_LEVEL_DEBUG, "Message Received!");

  if (price_tuple) {
    strncpy(ticker_text, price_tuple->value->cstring, 14);
		text_layer_set_text(text_ticker_layer, ticker_text);
  }
	if (price_london_tuple) {
		strncpy(ticker_london_text, price_london_tuple->value->cstring, 15);
		text_layer_set_text(text_ticker_london_layer, ticker_london_text);
	}
}

static void in_dropped_handler(AppMessageResult reason, void *context)
{
  APP_LOG(APP_LOG_LEVEL_DEBUG, "App Message Dropped! %d", reason);
}

static void out_sent_handler(DictionaryIterator *sent, void *context)
{
  APP_LOG(APP_LOG_LEVEL_DEBUG, "App Message sent!");
}

static void out_failed_handler(DictionaryIterator *failed, AppMessageResult reason, void *context)
{
  APP_LOG(APP_LOG_LEVEL_DEBUG, "App Message Failed to Send! %x", reason);
	fetch_price();
}

static void app_message_init(void)
{
  // Register message handlers
  app_message_register_inbox_received(in_received_handler);
  app_message_register_inbox_dropped(in_dropped_handler);
	app_message_register_outbox_sent(out_sent_handler);
  app_message_register_outbox_failed(out_failed_handler);
  // Init buffers
  app_message_open(64, 64);
}


int main(void) {
  handle_init();

  app_event_loop();
  
  handle_deinit();
}
