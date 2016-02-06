#include <pebble.h>
#define KEY_TEMPERATURE 0
#define KEY_CONDITIONS 1
#define KEY_INVERT 2
#define KEY_TWENTY_FOUR_HOUR_FORMAT 3
#define KEY_CELSIUS 4

static Window *s_main_window;
// Font
static TextLayer *s_time_layer;
static GFont s_time_font;
// Weather
static TextLayer *s_weather_layer;
static GFont s_weather_font;
// Date
static TextLayer *s_date_layer;
static GFont s_date_font;
// Battery
static int s_battery_level;
static Layer *s_battery_layer;
// Steps
int s_steps;
static TextLayer *s_num_label;

static bool twenty_four_hour_format = false;
static bool celsius = false;
static bool invertColors = false;


static void update_time() {
  // Get a tm structure
  time_t temp = time(NULL);
  struct tm *tick_time = localtime(&temp);

  // Write the current hours and minutes into a buffer
  static char s_buffer[8];
  strftime(s_buffer, sizeof(s_buffer), "%I:%M", tick_time);

  // Display this time on the TextLayer
  if('0' == s_buffer[0]) {
    text_layer_set_text(s_time_layer, s_buffer+1);
  } else
  {
  text_layer_set_text(s_time_layer, s_buffer);
  }
  
  // Copy date into buffer from tm structure
  static char date_buffer[16];
  strftime(date_buffer, sizeof(date_buffer), PBL_IF_ROUND_ELSE("%a  %b %d", "%a %n %b %d"), tick_time);

  // Show the date
  int x;
  PBL_IF_ROUND_ELSE(x = 9, x = 10);
  if(date_buffer[x]=='0') {
  date_buffer[x]=date_buffer[x+1]; // copy the second digit on top of the first one
  date_buffer[x+1]=0; // shorten the string by one character
}
  text_layer_set_text(s_date_layer, date_buffer);
}

static void invert() {
  GColor background_color = GColorWhite;
  window_set_background_color(s_main_window, background_color);
  text_layer_set_text_color(s_time_layer, GColorWhite);
  text_layer_set_text_color(s_date_layer, GColorWhite);
  text_layer_set_text_color(s_weather_layer, GColorWhite);
  text_layer_set_text_color(s_num_label, GColorWhite);
}
/*
static void inbox_received_handler(DictionaryIterator *iter, void *context) {
  Tuple *invert_t = dict_find(iter, KEY_INVERT);
  Tuple *twenty_four_hour_format_t = dict_find(iter, KEY_TWENTY_FOUR_HOUR_FORMAT);
  Tuple *celsius_t = dict_find(iter, KEY_CELSIUS);

  if (invert_t) {
    invertColors = invert_t->value->int8;

    persist_write_int(KEY_INVERT, invertColors);

    invert();
  }

  if (twenty_four_hour_format_t) {
    twenty_four_hour_format = twenty_four_hour_format_t->value->int8;

    persist_write_int(KEY_TWENTY_FOUR_HOUR_FORMAT, twenty_four_hour_format);

    update_time();
  }
  
  if (celsius_t) {
    celsius = celsius_t->value->int8;

    persist_write_int(KEY_CELSIUS, celsius);

    update_time();
  }
}
*/

static void tick_handler(struct tm *tick_time, TimeUnits units_changed) {
  update_time();
  // Get weather update every 30 minutes
  if(tick_time->tm_min % 30 == 0) {
    // Begin dictionary
    DictionaryIterator *iter;
    app_message_outbox_begin(&iter);
  
    // Add a key-value pair
    dict_write_uint8(iter, 0, 0);
  
    // Send the message!
    app_message_outbox_send();
  }
}

static void battery_callback(BatteryChargeState state) {
  // Record the new battery level
  s_battery_level = state.charge_percent;
  // Update meter
  layer_mark_dirty(s_battery_layer);
}

static void battery_update_proc(Layer *layer, GContext *ctx) {
  GRect bounds = layer_get_bounds(layer);

  // Find the width of the bar
  int width = (int)(float)(((float)s_battery_level / 100.0F) * 114.0F);

  // Draw the background
  graphics_context_set_fill_color(ctx, GColorClear);
  graphics_fill_rect(ctx, bounds, 0, GCornerNone);

  // Draw the bar
  if (persist_read_bool(KEY_INVERT)) {
    graphics_context_set_fill_color(ctx, GColorWhite);
  } else {
    graphics_context_set_fill_color(ctx, GColorBlack);
  }
  if (s_battery_level <= 20) {
    graphics_context_set_fill_color(ctx, GColorRed);
  } else if (s_battery_level == 100) {
    graphics_context_set_fill_color(ctx, GColorBrightGreen);
  }
  //graphics_context_set_fill_color(ctx, GColorBlack);
  graphics_fill_rect(ctx, GRect(0, 0, width, bounds.size.h), 0, GCornerNone);
}


static void health_handler(HealthEventType event, void *context) {
  static char s_value_buffer[8];
  if (event == HealthEventMovementUpdate) {
    // display the step count
    snprintf(s_value_buffer, sizeof(s_value_buffer), "%d", (int)health_service_sum_today(HealthMetricStepCount));
    text_layer_set_text(s_num_label, s_value_buffer);
  }
}


static void main_window_load(Window *window) {
  // Get information about the Window
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);

  // Time Layer
  s_time_layer = text_layer_create(GRect(0, PBL_IF_ROUND_ELSE(52, 46), bounds.size.w, 80));
  s_time_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_SF_55));
  text_layer_set_font(s_time_layer, s_time_font);
  text_layer_set_background_color(s_time_layer, GColorClear);
  
  if (persist_read_bool(KEY_INVERT)) {
    invertColors = persist_read_int(KEY_INVERT);
    invert();
  } else {
    text_layer_set_text_color(s_time_layer, GColorBlack);
  }

  if (persist_read_bool(KEY_TWENTY_FOUR_HOUR_FORMAT)) {
    twenty_four_hour_format = persist_read_bool(KEY_TWENTY_FOUR_HOUR_FORMAT);
  }
  
  if (persist_read_bool(KEY_CELSIUS)) {
    celsius = persist_read_int(KEY_CELSIUS);
  }
  text_layer_set_text_alignment(s_time_layer, GTextAlignmentCenter);

  // Add Time Layer to Window
  layer_add_child(window_layer, text_layer_get_layer(s_time_layer));
  
  // Date Layer
  s_date_layer = text_layer_create(GRect(PBL_IF_ROUND_ELSE(52, 0), PBL_IF_ROUND_ELSE(15, 1), PBL_IF_ROUND_ELSE(75, 45), 45));
  s_date_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_SF_17));
  //text_layer_set_text_color(s_date_layer, GColorBlack);
  text_layer_set_background_color(s_date_layer, GColorClear);
  text_layer_set_text_alignment(s_date_layer, GTextAlignmentCenter);
  text_layer_set_font(s_date_layer, s_date_font);
  
  // Add Date Layer to Window
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_date_layer));
  
  // Weather Layer
  s_weather_layer = text_layer_create(GRect(0, PBL_IF_ROUND_ELSE(125, 120), bounds.size.w, 25));
  text_layer_set_background_color(s_weather_layer, GColorClear);
  //text_layer_set_text_color(s_weather_layer, GColorBlack);
  text_layer_set_text_alignment(s_weather_layer, GTextAlignmentCenter);
  text_layer_set_text(s_weather_layer, "Loading...");
  s_weather_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_SF_20));
  text_layer_set_font(s_weather_layer, s_weather_font);
  
  // Add Weather Layer to Window
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_weather_layer));
  
  // Battery Layer
  s_battery_layer = layer_create(GRect(PBL_IF_ROUND_ELSE(40, 20), 154, PBL_IF_ROUND_ELSE(100, 104), 2));
  layer_set_update_proc(s_battery_layer, battery_update_proc);

  // Add Battery Layer to Window
  layer_add_child(window_get_root_layer(window), s_battery_layer);
  
  // Steps Layer
  s_num_label = text_layer_create(GRect(PBL_IF_ROUND_ELSE(67, 90), PBL_IF_ROUND_ELSE(37, 1), 50, 45));
  text_layer_set_background_color(s_num_label, GColorClear);
  //text_layer_set_text_color(s_num_label, GColorBlack);
  text_layer_set_text_alignment(s_num_label, PBL_IF_ROUND_ELSE(GTextAlignmentCenter, GTextAlignmentRight));
  text_layer_set_font(s_num_label, fonts_load_custom_font(resource_get_handle(RESOURCE_ID_SF_17)));
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_num_label));
  
  // subscribe to health events
  if(health_service_events_subscribe(health_handler, NULL)) {
    // force initial steps display
    health_handler(HealthEventMovementUpdate, NULL);
  } else {
    APP_LOG(APP_LOG_LEVEL_ERROR, "Health not available!");
  }
}

static void main_window_unload(Window *window) {
  // Destroy TextLayer
  text_layer_destroy(s_time_layer);
  // Unload GFont
  fonts_unload_custom_font(s_time_font);
  // Destroy weather elements
  text_layer_destroy(s_weather_layer);
  fonts_unload_custom_font(s_weather_font);
  fonts_unload_custom_font(s_date_font);
  text_layer_destroy(s_date_layer);
  layer_destroy(s_battery_layer);
  text_layer_destroy(s_num_label);

}

static void inbox_received_callback(DictionaryIterator *iterator, void *context) {
  // Store incoming information
  static char temperature_buffer[8];
  static char conditions_buffer[32];
  static char weather_layer_buffer[32];
  
  Tuple *invert_t = dict_find(iterator, KEY_INVERT);
  Tuple *twenty_four_hour_format_t = dict_find(iterator, KEY_TWENTY_FOUR_HOUR_FORMAT);
  Tuple *celsius_t = dict_find(iterator, KEY_CELSIUS);

  if (invert_t) {
    invertColors = invert_t->value->int8;

    persist_write_int(KEY_INVERT, invertColors);

    invert();
  }

  if (twenty_four_hour_format_t) {
    twenty_four_hour_format = twenty_four_hour_format_t->value->int8;

    persist_write_int(KEY_TWENTY_FOUR_HOUR_FORMAT, twenty_four_hour_format);

    update_time();
  }
  
  if (celsius_t) {
    celsius = celsius_t->value->int8;

    persist_write_int(KEY_CELSIUS, celsius);

    update_time();
  }
  
  // Read tuples for data
  Tuple *temp_tuple = dict_find(iterator, KEY_TEMPERATURE);
  Tuple *conditions_tuple = dict_find(iterator, KEY_CONDITIONS);
  int temp = temp_tuple->value->int32;
  if(!(celsius)) {
    temp = (int)(temp * 5.0/9.0 + 32);
  }
  
  // If all data is available, use it
  if(temp_tuple && conditions_tuple) {
    snprintf(temperature_buffer, sizeof(temperature_buffer), "%dº", temp);
    snprintf(conditions_buffer, sizeof(conditions_buffer), "%s", conditions_tuple->value->cstring);
  } 
    // Assemble full string and display
    snprintf(weather_layer_buffer, sizeof(weather_layer_buffer), "%s, %s", temperature_buffer, conditions_buffer);
    text_layer_set_text(s_weather_layer, weather_layer_buffer);
}

static void inbox_dropped_callback(AppMessageResult reason, void *context) {
  APP_LOG(APP_LOG_LEVEL_ERROR, "Message dropped!");
}

static void outbox_failed_callback(DictionaryIterator *iterator, AppMessageResult reason, void *context) {
  APP_LOG(APP_LOG_LEVEL_ERROR, "Outbox send failed!");
}

static void outbox_sent_callback(DictionaryIterator *iterator, void *context) {
  APP_LOG(APP_LOG_LEVEL_INFO, "Outbox send success!");
}


static void init() {
  // Create main Window element and assign to pointer
  s_main_window = window_create();

  
  
  // Set handlers to manage the elements inside the Window
  window_set_window_handlers(s_main_window, (WindowHandlers) {
    .load = main_window_load,
    .unload = main_window_unload
  });

  // Show the Window on the watch, with animated=true
  window_stack_push(s_main_window, true);
  
  // Register with TickTimerService
  tick_timer_service_subscribe(MINUTE_UNIT, tick_handler);
  
  // Register for battery level updates
  battery_state_service_subscribe(battery_callback);
  
  // Make sure the time is displayed from the start
  update_time();
  
  // Ensure battery level is displayed from the start
  battery_callback(battery_state_service_peek());
  
  // Register callbacks
  app_message_register_inbox_received(inbox_received_callback);
  
  // Open AppMessage
  app_message_open(app_message_inbox_size_maximum(), app_message_outbox_size_maximum());
  
  app_message_register_inbox_dropped(inbox_dropped_callback);
  app_message_register_outbox_failed(outbox_failed_callback);
  app_message_register_outbox_sent(outbox_sent_callback);

}

static void deinit() {
  // Destroy Window
  window_destroy(s_main_window);
}

int main(void) {
  init();
  app_event_loop();
  deinit();
}

