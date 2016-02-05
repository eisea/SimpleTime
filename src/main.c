#include <pebble.h>
#define KEY_TEMPERATURE 0
#define KEY_CONDITIONS 1
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
/* Steps
int s_steps;
static TextLayer *s_steps_layer;
static GFont s_steps_font;
*/

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
  strftime(date_buffer, sizeof(date_buffer), "%a %n %b %d", tick_time);

  // Show the date
  if(date_buffer[10]=='0') {
  date_buffer[10]=date_buffer[11]; // copy the second digit on top of the first one
  date_buffer[11]=0; // shorten the string by one character
}
  text_layer_set_text(s_date_layer, date_buffer);
}

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
  graphics_context_set_fill_color(ctx, GColorWhite);
  graphics_fill_rect(ctx, bounds, 0, GCornerNone);

  // Draw the bar
  graphics_context_set_fill_color(ctx, GColorBlack);
  graphics_fill_rect(ctx, GRect(0, 0, width, bounds.size.h), 0, GCornerNone);
  graphics_fill_rect(ctx, GRect(0, 0, width, bounds.size.h), 0, GCornerNone);
}


/*
static void steps_callback(int steps) {
  s_steps = steps;
}


static void health_handler(HealthEventType event, void *context) {
  // Which type of event occured?
  switch(event) {
    case HealthEventSignificantUpdate:
      APP_LOG(APP_LOG_LEVEL_INFO, 
              "New HealthService HealthEventSignificantUpdate event");
      break;
    case HealthEventMovementUpdate:
      APP_LOG(APP_LOG_LEVEL_INFO, 
              "New HealthService HealthEventMovementUpdate event");
      break;
    case HealthEventSleepUpdate:
      APP_LOG(APP_LOG_LEVEL_INFO, 
              "New HealthService HealthEventSleepUpdate event");
      break;
  }
  
}
*/

static void main_window_load(Window *window) {
  // Get information about the Window
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);

  // Time Layer
  s_time_layer = text_layer_create(GRect(0, PBL_IF_ROUND_ELSE(52, 46), bounds.size.w, 80));
  s_time_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_SF_55));
  text_layer_set_font(s_time_layer, s_time_font);
  text_layer_set_background_color(s_time_layer, GColorClear);
  text_layer_set_text_color(s_time_layer, GColorBlack);
  text_layer_set_text_alignment(s_time_layer, GTextAlignmentCenter);

  // Add Time Layer to Window
  layer_add_child(window_layer, text_layer_get_layer(s_time_layer));
  
  // Date Layer
  s_date_layer = text_layer_create(GRect(PBL_IF_ROUND_ELSE(62, 0), PBL_IF_ROUND_ELSE(15, 1), 55, 45));
  s_date_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_SF_17));
  text_layer_set_text_color(s_date_layer, GColorBlack);
  text_layer_set_background_color(s_date_layer, GColorClear);
  text_layer_set_text_alignment(s_date_layer, GTextAlignmentCenter);
  text_layer_set_font(s_date_layer, s_date_font);
  
  // Add Date Layer to Window
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_date_layer));
  
  // Weather Layer
  s_weather_layer = text_layer_create(GRect(0, PBL_IF_ROUND_ELSE(125, 120), bounds.size.w, 25));
  text_layer_set_background_color(s_weather_layer, GColorClear);
  text_layer_set_text_color(s_weather_layer, GColorBlack);
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
  
  /*
  // Steps Layer
  s_steps_layer = text_layer_create(GRect(50, 1, 55, 45));
  s_steps_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_SF_17));
  text_layer_set_text_color(s_steps_layer, GColorBlack);
  text_layer_set_background_color(s_steps_layer, GColorClear);
  text_layer_set_text_alignment(s_steps_layer, GTextAlignmentCenter);
  text_layer_set_font(s_date_layer, s_steps_font);
  printf("%d", s_steps);
  
  // Add Steps Layer to Window
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_steps_layer));
  */
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
  //text_layer_destroy(s_steps_layer);

}

static void inbox_received_callback(DictionaryIterator *iterator, void *context) {
  // Store incoming information
  static char temperature_buffer[8];
  static char conditions_buffer[32];
  static char weather_layer_buffer[32];
  
  // Read tuples for data
  Tuple *temp_tuple = dict_find(iterator, KEY_TEMPERATURE);
  Tuple *conditions_tuple = dict_find(iterator, KEY_CONDITIONS);
  
  // If all data is available, use it
  if(temp_tuple && conditions_tuple) {
    snprintf(temperature_buffer, sizeof(temperature_buffer), "%dº", (int)temp_tuple->value->int32);
    snprintf(conditions_buffer, sizeof(conditions_buffer), "%s", conditions_tuple->value->cstring);
    
    // Assemble full string and display
    snprintf(weather_layer_buffer, sizeof(weather_layer_buffer), "%s, %s", temperature_buffer, conditions_buffer);
    text_layer_set_text(s_weather_layer, weather_layer_buffer);
  }
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
  
  
  /*
  
  
  #if defined(PBL_HEALTH)
  // Attempt to subscribe 
  if(health_service_events_subscribe(health_handler, NULL)) {
    HealthMetric metric = HealthMetricStepCount;
    time_t start = time_start_of_today();
    time_t end = time(NULL);
    HealthServiceAccessibilityMask mask = health_service_metric_accessible(metric, 
                                                                      start, end);
    bool any_data_available = mask & HealthServiceAccessibilityMaskAvailable;
    if(any_data_available)
      {
      APP_LOG(APP_LOG_LEVEL_INFO, "Steps today: %d", 
            (int)health_service_sum_today(metric));
      steps_callback((int)health_service_sum_today(metric));
      s_steps = (int)health_service_sum_today(metric);
      printf("%d", s_steps);
      
      char str[10];
      snprintf(str, 10, "%d", s_steps);
      printf(str);
      text_layer_set_text(s_steps_layer, str);
      HealthActivityMask activities = health_service_peek_current_activities();
      if(activities & HealthActivitySleep) {
        APP_LOG(APP_LOG_LEVEL_INFO, "The user is sleeping.");
      } else if(activities & HealthActivityRestfulSleep) {
        APP_LOG(APP_LOG_LEVEL_INFO, "The user is sleeping peacefully.");
      } else {
        APP_LOG(APP_LOG_LEVEL_INFO, "The user is not currently sleeping.");
      }
    }
    
    
  }
  #else
  APP_LOG(APP_LOG_LEVEL_ERROR, "Health not available!");
  #endif
  
  */
  
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