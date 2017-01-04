#include <pebble.h>
#include <math.h>

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
// Bluetooth
static Layer *s_bt_icon_layer;
static GBitmap *s_bt_icon_bitmap;
int min_count = 0;
static bool celsius = false;

unsigned long createRGB(int r, int g, int b)
{   
    return ((r & 0xff) << 16) + ((g & 0xff) << 8) + (b & 0xff);
}

static void update_time() {
  // Get a tm structure
  time_t temp = time(NULL);
  struct tm *tick_time = localtime(&temp);

  // Write the current hours and minutes into a buffer
  static char s_buffer[8];
  APP_LOG(APP_LOG_LEVEL_INFO, "24 hour time: %d", persist_read_bool(MESSAGE_KEY_Twenty_Four_Hour_Format));
  if (persist_read_bool(MESSAGE_KEY_Twenty_Four_Hour_Format) == 1) {
    APP_LOG(APP_LOG_LEVEL_INFO, "24h:");
    strftime(s_buffer, sizeof("00:00"), "%H:%M", tick_time);
  } else {
  APP_LOG(APP_LOG_LEVEL_INFO, "12h:");
  strftime(s_buffer, sizeof(s_buffer), "%I:%M", tick_time);
  }
  // Display this time on the TextLayer
  if('0' == s_buffer[0]) {
    text_layer_set_text(s_time_layer, s_buffer+1);
  } else
  {
  text_layer_set_text(s_time_layer, s_buffer);
  }
  APP_LOG(APP_LOG_LEVEL_INFO, "getting date");
  // Copy date into buffer from tm structure
  static char date_buffer[16];
  strftime(date_buffer, sizeof(date_buffer), PBL_IF_BW_ELSE("%a  %b %d", PBL_IF_ROUND_ELSE("%a  %b %d", "%a %n %b %d")), tick_time);
  APP_LOG(APP_LOG_LEVEL_INFO, "date calculations");
  // Show the date
  int x;
  PBL_IF_BW_ELSE(x = 9, x = 10);
  PBL_IF_ROUND_ELSE(x = 9, x = 10);
  if(date_buffer[x]=='0') {
  date_buffer[x]=date_buffer[x+1]; // copy the second digit on top of the first one
  date_buffer[x+1]=0; // shorten the string by one character
}
  APP_LOG(APP_LOG_LEVEL_INFO, "set date text:");
  text_layer_set_text(s_date_layer, date_buffer);
  APP_LOG(APP_LOG_LEVEL_INFO, "returning from updatetime");
}

static void tick_handler(struct tm *tick_time, TimeUnits units_changed) {
  update_time();
  /*if(units_changed = DAY_UNIT) {
    vibe = 0;
  }*/  
  
  min_count = min_count+1;
  
  // Get weather update every n minutes
  int fetch = persist_read_int(MESSAGE_KEY_Weather_Fetch);
  
  APP_LOG(APP_LOG_LEVEL_INFO, "count=%d", min_count);
  APP_LOG(APP_LOG_LEVEL_INFO, "fetch=%d", fetch);
  
  //if(tick_time->tm_min % fetch == 0) {
  if(min_count > fetch) {
    min_count = 0;
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

/*

static void accel_tap_handler(AccelAxisType axis, int32_t direction) {
  // A tap event occured
  APP_LOG(APP_LOG_LEVEL_INFO, "shake registered");
}

*/

static void layer_update_proc(Layer *layer, GContext *ctx) {
  // Draw the image with the correct compositing mode
  graphics_context_set_compositing_mode(ctx, GCompOpSet);
  graphics_draw_bitmap_in_rect(ctx, s_bt_icon_bitmap, gbitmap_get_bounds(s_bt_icon_bitmap));
}

static void battery_update_proc(Layer *layer, GContext *ctx) {
  GRect bounds = layer_get_bounds(layer);

  // Find the width of the bar
  float percent = ((float)s_battery_level / 100.0F);
  int width = (int)(float)(percent * bounds.size.w);

  // Draw the background
  
  GColor bgcolor = GColorFromHEX(persist_read_int(MESSAGE_KEY_BackgroundColor));
  GColor batterycolor;
  
  //graphics_context_set_fill_color(ctx, GColorClear);
  //graphics_fill_rect(ctx, bounds, 0, GCornerNone);

  // Draw the bar
  int g = (255 * percent);
  int r = (255 * (1 - percent));
  int b = 0;
  int rgb = createRGB(r, g, b);

  if (s_battery_level == 100) {
    r = 85;
    g = 255;
  }
  if (s_battery_level == 50) {
    rgb = 0xFFAA00;
  }
  int x = persist_read_int(MESSAGE_KEY_Battery);
  APP_LOG(APP_LOG_LEVEL_INFO, "Battery Preference: %d", x);
  if (persist_read_int(MESSAGE_KEY_Battery) != 102) {
    APP_LOG(APP_LOG_LEVEL_INFO, "RGB");
    batterycolor = GColorFromHEX(rgb);
  } else {
    batterycolor = gcolor_legible_over(bgcolor);
  }

  PBL_IF_BW_ELSE(graphics_context_set_fill_color(ctx, batterycolor), graphics_context_set_fill_color(ctx, batterycolor));
  //APP_LOG(APP_LOG_LEVEL_INFO, GColorFromHEX(rgb));
  
  /*
  if (s_battery_level <= 20) {
    graphics_context_set_fill_color(ctx, PBL_IF_BW_ELSE(GColorBlack, GColorRed));
  } else if (s_battery_level == 100) {
    graphics_context_set_fill_color(ctx, PBL_IF_BW_ELSE(GColorBlack, GColorBrightGreen));
  }
  */
  
  GRect frame = grect_inset(bounds, GEdgeInsets(10));
  
  //graphics_context_set_fill_color(ctx, GColorBlack);
  
  PBL_IF_ROUND_ELSE(graphics_fill_radial(ctx, frame, GOvalScaleModeFitCircle, 5,
                       DEG_TO_TRIGANGLE(125+((1-percent)*110)), DEG_TO_TRIGANGLE(235)), 
                    graphics_fill_rect(ctx, GRect(0, 0, width, bounds.size.h), 0, GCornerNone));
  
}

static void bluetooth_callback(bool connected) {  
  // Show icon if disconnected
  layer_set_hidden(s_bt_icon_layer, connected);
  if (connected) {
    APP_LOG(APP_LOG_LEVEL_INFO, "BT reconnected, calling weather");
    DictionaryIterator *iter;
    app_message_outbox_begin(&iter);
  
    // Add a key-value pair
    dict_write_uint8(iter, 0, 0);
  
    // Send the message!
    app_message_outbox_send();
  }
}


static void health_handler(HealthEventType event, void *context) {
  static char s_value_buffer[8];
  if (event == HealthEventMovementUpdate) {
    if (persist_exists(MESSAGE_KEY_Step_Goal)) {
      int steps_color = persist_read_int(MESSAGE_KEY_Goal_Color);
      int goal_num = persist_read_int(MESSAGE_KEY_Step_Goal);
      GColor goal_color = GColorFromHEX(steps_color);
      APP_LOG(APP_LOG_LEVEL_INFO, "Steps today (health handler): %d", (int)health_service_sum_today(HealthMetricStepCount));
      APP_LOG(APP_LOG_LEVEL_INFO, "Goal Steps (health handler): %d", goal_num);
      if (!(goal_num == 0 || (int)health_service_sum_today(HealthMetricStepCount) < goal_num)) {
        text_layer_set_text_color(s_num_label, goal_color);
        APP_LOG(APP_LOG_LEVEL_ERROR, "Steps color is Goal Color");
        if (persist_exists(MESSAGE_KEY_vibe)) {
          int vibe1 = persist_read_int(MESSAGE_KEY_vibe);
          if (vibe1 < 1){
            vibes_short_pulse();
            APP_LOG(APP_LOG_LEVEL_INFO, "Goal Reached! Vibrating...");
            vibe1 = 1;
            persist_write_int(MESSAGE_KEY_vibe, 1);
          }
        }
      }
    }
    // display the step count
    snprintf(s_value_buffer, sizeof(s_value_buffer), "%d", (int)health_service_sum_today(HealthMetricStepCount));
    text_layer_set_text(s_num_label, s_value_buffer);
  }
  else if (event == HealthEventSignificantUpdate) {
    persist_write_int(MESSAGE_KEY_vibe, 0);
    snprintf(s_value_buffer, sizeof(s_value_buffer), "%d", (int)health_service_sum_today(HealthMetricStepCount));
    text_layer_set_text_color(s_num_label, GColorFromHEX(persist_read_int(MESSAGE_KEY_TextColor)));
    text_layer_set_text(s_num_label, s_value_buffer);
  }
}

static void main_window_load(Window *window) {
  // Get information about the Window
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);

  

  APP_LOG(APP_LOG_LEVEL_INFO, "Main window loading");
  
  APP_LOG(APP_LOG_LEVEL_INFO, "battery time");

  s_battery_layer = layer_create(GRect(PBL_IF_ROUND_ELSE(0, 20), PBL_IF_ROUND_ELSE(0, 154), PBL_IF_ROUND_ELSE(180, 104), PBL_IF_ROUND_ELSE(180, 2)));
  s_weather_layer = text_layer_create(GRect(0, PBL_IF_ROUND_ELSE(117, 120), bounds.size.w, 55));
  s_time_layer = text_layer_create(GRect(0, PBL_IF_ROUND_ELSE(52, 46), bounds.size.w, 80));
  s_date_layer = PBL_IF_BW_ELSE(text_layer_create(GRect(1, 2, bounds.size.w, 50)), text_layer_create(GRect(PBL_IF_ROUND_ELSE(42, 1), PBL_IF_ROUND_ELSE(15, 1), PBL_IF_ROUND_ELSE(97, 65), 55)));
  s_bt_icon_layer = PBL_IF_BW_ELSE(layer_create(GRect(60, 115, 30, 30)), layer_create(GRect(PBL_IF_ROUND_ELSE(75, 62), PBL_IF_ROUND_ELSE(135, 8), 30, 30)));
  
  
  int textcolor = persist_read_int(MESSAGE_KEY_TextColor);
  int background = persist_read_int(MESSAGE_KEY_BackgroundColor);
  GColor bg_color = GColorFromHEX(background);
  GColor text_color = GColorFromHEX(textcolor);
  window_set_background_color(s_main_window, bg_color);
  APP_LOG(APP_LOG_LEVEL_INFO, "Text Color is: %d", textcolor);
  APP_LOG(APP_LOG_LEVEL_INFO, "Background Color is: %d", background);
  
  
  // Battery Layer
  layer_set_update_proc(s_battery_layer, battery_update_proc);

  // Add Battery Layer to Window
  layer_add_child(window_get_root_layer(window), s_battery_layer);
  
  

  
  APP_LOG(APP_LOG_LEVEL_INFO, "reading key: celsius");
  
  if (persist_read_bool(MESSAGE_KEY_Celsius)) {
    celsius = persist_read_int(MESSAGE_KEY_Celsius);
  }
  
  /*
  APP_LOG(APP_LOG_LEVEL_INFO, "reading key: customLocation");
  if (persist_read_string(KEY_CUSTOM_LOCATION)) {
    customLocation = persist_read_string(KEY_CUSTOM_LOCATION);
  }*/
  
  APP_LOG(APP_LOG_LEVEL_INFO, "weather time");
  
  
  
  // Weather Layer
  text_layer_set_background_color(s_weather_layer, GColorClear);
  text_layer_set_text_color(s_weather_layer, text_color);
  //text_layer_set_text_color(s_weather_layer, GColorBlack);
  text_layer_set_text_alignment(s_weather_layer, GTextAlignmentCenter);
  PBL_IF_BW_ELSE(text_layer_set_text(s_weather_layer, ""), text_layer_set_text(s_weather_layer, "Loading..."));
  s_weather_font = PBL_IF_ROUND_ELSE(fonts_load_custom_font(resource_get_handle(RESOURCE_ID_SF_17)), fonts_load_custom_font(resource_get_handle(RESOURCE_ID_SF_20)));
  text_layer_set_font(s_weather_layer, s_weather_font);
  
  // Add Weather Layer to Window
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_weather_layer));

  APP_LOG(APP_LOG_LEVEL_INFO, "time layer");
  
 
  
  
  // Time Layer
  s_time_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_SF_53));
  text_layer_set_font(s_time_layer, s_time_font);
  text_layer_set_background_color(s_time_layer, GColorClear);
  text_layer_set_text_color(s_time_layer, text_color);
  
  APP_LOG(APP_LOG_LEVEL_INFO, "reading key: invert");

  APP_LOG(APP_LOG_LEVEL_INFO, "reading key: 24h");
  

  
  text_layer_set_text_alignment(s_time_layer, GTextAlignmentCenter);

  // Add Time Layer to Window
  layer_add_child(window_layer, text_layer_get_layer(s_time_layer));
  
  APP_LOG(APP_LOG_LEVEL_INFO, "date time");

  // Date Layer
  s_date_font = PBL_IF_BW_ELSE(fonts_load_custom_font(resource_get_handle(RESOURCE_ID_SF_20)), fonts_load_custom_font(resource_get_handle(RESOURCE_ID_SF_17)));
	text_layer_set_text_color(s_date_layer, text_color);
  //text_layer_set_text_color(s_date_layer, GColorBlack);
  text_layer_set_background_color(s_date_layer, GColorClear);
  text_layer_set_text_alignment(s_date_layer, GTextAlignmentCenter);
  text_layer_set_font(s_date_layer, s_date_font);

  APP_LOG(APP_LOG_LEVEL_INFO, "updating time");
  
  
  // Add Date Layer to Window
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_date_layer));
  
  
  // Create the Bluetooth icon GBitmap
  APP_LOG(APP_LOG_LEVEL_INFO, "Reading Bluetooth color: %d", persist_read_bool(MESSAGE_KEY_Bluetooth));
  if (persist_read_int(MESSAGE_KEY_Bluetooth) == 102) {
    s_bt_icon_bitmap = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_BT_ICON);
  } else {
    s_bt_icon_bitmap = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_BT_ICON_BLACK);
  }
  // Create the BitmapLayer to display the GBitmap
  s_bt_icon_layer = PBL_IF_BW_ELSE(layer_create(GRect(60, 115, 30, 30)), layer_create(GRect(PBL_IF_ROUND_ELSE(75, 62), PBL_IF_ROUND_ELSE(135, 8), 30, 30)));
  layer_set_update_proc(s_bt_icon_layer, layer_update_proc);
  //bitmap_layer_set_bitmap(s_bt_icon_layer, s_bt_icon_bitmap);
  layer_add_child(window_layer, s_bt_icon_layer);
  
  APP_LOG(APP_LOG_LEVEL_INFO, "steps time");

  // Steps Layer
  
  // subscribe to health events
  int goal_num = persist_read_int(MESSAGE_KEY_Step_Goal);
  int steps = persist_read_int(MESSAGE_KEY_Goal_Color);
  GColor steps_color = GColorFromHEX(steps);
  s_num_label = text_layer_create(GRect(PBL_IF_ROUND_ELSE(67, 85), PBL_IF_ROUND_ELSE(37, 1), PBL_IF_ROUND_ELSE(50, 55), 45));
  text_layer_set_background_color(s_num_label, GColorClear);
  text_layer_set_text_color(s_num_label, text_color);
  APP_LOG(APP_LOG_LEVEL_ERROR, "Steps color is Text Color");
  int s_step_count = (int)health_service_sum_today(HealthMetricStepCount);
  APP_LOG(APP_LOG_LEVEL_ERROR, "goal: %d", goal_num);
  APP_LOG(APP_LOG_LEVEL_ERROR, "steps: %d", s_step_count);
  if (goal_num == 0 || s_step_count < goal_num) {
    int textcolor = persist_read_int(MESSAGE_KEY_TextColor);
    GColor text_color = GColorFromHEX(textcolor);
    text_layer_set_text_color(s_num_label, text_color);
    APP_LOG(APP_LOG_LEVEL_ERROR, "Steps color is Text Color");
  }
  else {
    text_layer_set_text_color(s_num_label, steps_color);
    APP_LOG(APP_LOG_LEVEL_ERROR, "Steps color is Goal Color");
  }
  
  //text_layer_set_text_color(s_num_label, GColorBlack);
  text_layer_set_text_alignment(s_num_label, PBL_IF_ROUND_ELSE(GTextAlignmentCenter, GTextAlignmentRight));
  text_layer_set_font(s_num_label, fonts_load_custom_font(resource_get_handle(RESOURCE_ID_SF_17)));
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_num_label));
  
  if(health_service_events_subscribe(health_handler, NULL)) {
    // force initial steps display
    health_handler(HealthEventMovementUpdate, NULL);
  } else {
    APP_LOG(APP_LOG_LEVEL_ERROR, "Health not available!");
  }
  
 
	
  
  // Show the correct state of the BT connection from the start
  bluetooth_callback(connection_service_peek_pebble_app_connection());


  APP_LOG(APP_LOG_LEVEL_INFO, "Main window loaded");
  
  update_time();
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
  gbitmap_destroy(s_bt_icon_bitmap);
  layer_destroy(s_bt_icon_layer);
  APP_LOG(APP_LOG_LEVEL_INFO, "Main window unloaded");

}

static void inbox_received_handler(DictionaryIterator *iterator, void *context) {
  // Store incoming information

  APP_LOG(APP_LOG_LEVEL_INFO, "inbox received");

  static char temperature_buffer[8];
  static char conditions_buffer[32];
  static char weather_layer_buffer[38];
  //static char location_layer_buffer[38];
  
  //Tuple *invert_t = dict_find(iterator, KEY_INVERT);
  APP_LOG(APP_LOG_LEVEL_INFO, "Reading 24 hour Tuple");
  Tuple *twenty_four_hour_format_t = dict_find(iterator, MESSAGE_KEY_Twenty_Four_Hour_Format);
  APP_LOG(APP_LOG_LEVEL_INFO, "Reading Celsius Tuple");
  Tuple *celsius_t = dict_find(iterator, MESSAGE_KEY_Celsius);
  APP_LOG(APP_LOG_LEVEL_INFO, "Reading Background color Tuple");
  Tuple *bg_color_t = dict_find(iterator, MESSAGE_KEY_BackgroundColor);
  if(bg_color_t) {
    //bg_color = GColorFromHEX(bg_color_t->value->int32);
    int background = bg_color_t->value->int32;
    persist_write_int(MESSAGE_KEY_BackgroundColor, background);
  }
  else if(!persist_exists(MESSAGE_KEY_BackgroundColor)) {
    persist_write_int(MESSAGE_KEY_BackgroundColor, 0xFFFFFF);
  }
  APP_LOG(APP_LOG_LEVEL_INFO, "Reading Text color Tuple");
  Tuple *fg_color_t = dict_find(iterator, MESSAGE_KEY_TextColor);
  if(fg_color_t) {
    //text_color = GColorFromHEX(fg_color_t->value->int32);
    int textColor = fg_color_t->value->int32;
    persist_write_int(MESSAGE_KEY_TextColor, textColor);
  }
  else if(!persist_exists(MESSAGE_KEY_TextColor)) {
    persist_write_int(MESSAGE_KEY_TextColor, 0x000000);
  }
  APP_LOG(APP_LOG_LEVEL_INFO, "Reading Battery Tuple");
  Tuple *battery_t = dict_find(iterator, MESSAGE_KEY_Battery);
  
  APP_LOG(APP_LOG_LEVEL_INFO, "Reading Bluetooth Tuple");
  Tuple *bluetooth_t = dict_find(iterator, MESSAGE_KEY_Bluetooth);
  //Tuple *custom_location_t = dict_find(iterator, KEY_CUSTOM_LOCATION);
  
  APP_LOG(APP_LOG_LEVEL_INFO, "Reading Weather Fetch Tuple");
  Tuple *weather_fetch_t = dict_find(iterator, MESSAGE_KEY_Weather_Fetch);
  
  APP_LOG(APP_LOG_LEVEL_INFO, "Reading Step Goal Tuple");
  Tuple *step_goal_t = dict_find(iterator, MESSAGE_KEY_Step_Goal);
  
  APP_LOG(APP_LOG_LEVEL_INFO, "Reading Goal Color Tuple");
  Tuple *goal_color_t = dict_find(iterator, MESSAGE_KEY_Goal_Color);
  
  
  GColor bg_color = GColorFromHEX(persist_read_int(MESSAGE_KEY_BackgroundColor));
  window_set_background_color(s_main_window, bg_color);
  GColor text_color = GColorFromHEX(persist_read_int(MESSAGE_KEY_TextColor));
  //text_layer_set_text_color(s_num_label, text_color);
	text_layer_set_text_color(s_date_layer, text_color);
  text_layer_set_text_color(s_weather_layer, text_color);
  text_layer_set_text_color(s_time_layer, text_color);
  
  
  
  if (twenty_four_hour_format_t) {
    
    persist_write_bool(MESSAGE_KEY_Twenty_Four_Hour_Format, twenty_four_hour_format_t->value->int8);
    update_time();
    
  }
  
  if (celsius_t) {
    celsius = celsius_t->value->int8;
    APP_LOG(APP_LOG_LEVEL_INFO, "celsius: %d", celsius);

    persist_write_int(MESSAGE_KEY_Celsius, celsius);
    
    // Begin dictionary
    DictionaryIterator *iter;
    app_message_outbox_begin(&iter);
  
    // Add a key-value pair
    dict_write_uint8(iter, 0, 0);
  
    // Send the message!
    app_message_outbox_send();
    
  }
  
  if (battery_t) {
    int battery_pref = battery_t->value->int8;
    persist_write_int(MESSAGE_KEY_Battery, battery_pref);
    layer_mark_dirty(s_battery_layer);
  }
  
  if (bluetooth_t) {
    int bt = bluetooth_t->value->int8;
    persist_write_int(MESSAGE_KEY_Bluetooth, bt);
  }
  
  if (weather_fetch_t) {
    int fetch = weather_fetch_t->value->int8;
    persist_write_int(MESSAGE_KEY_Weather_Fetch, fetch);
  }
  
  //STEP GOALS
  if (step_goal_t) {
    int step_goal = step_goal_t->value->int32;
    persist_write_int(MESSAGE_KEY_Step_Goal, step_goal);
  }
  
  if (goal_color_t) {
    int steps_color = goal_color_t->value->int32;
    persist_write_int(MESSAGE_KEY_Goal_Color, steps_color);
    int goal_num = persist_read_int(MESSAGE_KEY_Step_Goal);
    GColor goal_color = GColorFromHEX(steps_color);
    APP_LOG(APP_LOG_LEVEL_INFO, "Steps today: %d", (int)health_service_sum_today(HealthMetricStepCount));
    APP_LOG(APP_LOG_LEVEL_INFO, "Goal Steps: %d", goal_num);
    if (goal_num == 0 || (int)health_service_sum_today(HealthMetricStepCount) < goal_num) {
      text_layer_set_text_color(s_num_label, text_color);
    } else {
      if (persist_exists(MESSAGE_KEY_vibe)){
        int vibe2 = persist_read_int(MESSAGE_KEY_vibe);
        if (vibe2 < 1){
          vibes_short_pulse();
          APP_LOG(APP_LOG_LEVEL_INFO, "Goal Reached! Vibrating...");
          vibe2 = 1;
          persist_write_int(MESSAGE_KEY_vibe, 1);
        }
        text_layer_set_text_color(s_num_label, goal_color);
      }
    }
  }
  else if(!persist_exists(MESSAGE_KEY_Goal_Color)) {
    persist_write_int(MESSAGE_KEY_Goal_Color, 0x000000);
  }
  
  
  /*
  if (custom_location_t) {
    customLocation = custom_location_t->value->cstring;
      
    persist_write_string(KEY_CUSTOM_LOCATION, customLocation);
    
    // Begin dictionary
    DictionaryIterator *iter;
    app_message_outbox_begin(&iter);
  
    // Add a key-value pair
    dict_write_uint8(iter, 0, 0);
  
    // Send the message!
    app_message_outbox_send();
  }
  */

  APP_LOG(APP_LOG_LEVEL_INFO, "%d", celsius);
  
  // Read tuples for data
  Tuple *temp_tuple = dict_find(iterator, MESSAGE_KEY_KEY_TEMPERATURE);
  Tuple *conditions_tuple = dict_find(iterator, MESSAGE_KEY_KEY_CONDITIONS);

  //APP_LOG(APP_LOG_LEVEL_INFO, "initializing temp");
  //temp = (int)temp_tuple->value->int32;
  //APP_LOG(APP_LOG_LEVEL_INFO, "temp initialized");
  //if(celsius == false) {
  //  temp = (int)(temp * 5.0/9.0 + 32);
  //}
  
  // If all data is available, use it
  
  
  
  if(temp_tuple && conditions_tuple && (celsius == 1)) {
    snprintf(temperature_buffer, sizeof(temperature_buffer), "%dº", (int)round((((float)temp_tuple->value->int32) - 32) * (5.0/9.0)));
    snprintf(conditions_buffer, sizeof(conditions_buffer), "%s", conditions_tuple->value->cstring);
  } else if(temp_tuple && conditions_tuple && (celsius == 0)) {
    snprintf(temperature_buffer, sizeof(temperature_buffer), "%dº", (int)(int)temp_tuple->value->int32);
    snprintf(conditions_buffer, sizeof(conditions_buffer), "%s", conditions_tuple->value->cstring);
  }
    
    // Assemble full string and display
    snprintf(weather_layer_buffer, sizeof(weather_layer_buffer), "%s, %s", temperature_buffer, conditions_buffer);
    //APP_LOG(APP_LOG_LEVEL_INFO, weather_layer_buffer);
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

  APP_LOG(APP_LOG_LEVEL_INFO, "window pushed");
  
  // Register with TickTimerService
  tick_timer_service_subscribe(MINUTE_UNIT, tick_handler);

  APP_LOG(APP_LOG_LEVEL_INFO, "Time subscribed");
  
  // Register for battery level updates
  battery_state_service_subscribe(battery_callback);
  
  /*
  
  // Subscribe to tap events
  accel_tap_service_subscribe(accel_tap_handler);

  */

  APP_LOG(APP_LOG_LEVEL_INFO, "Battery subscribed");
  
  // Make sure the time is displayed from the start
  update_time();
  APP_LOG(APP_LOG_LEVEL_INFO, "Time updated");
  
  // Ensure battery level is displayed from the start
  battery_callback(battery_state_service_peek());
  
  // Register for Bluetooth connection updates
  connection_service_subscribe((ConnectionHandlers) {
    .pebble_app_connection_handler = bluetooth_callback
  });

  APP_LOG(APP_LOG_LEVEL_INFO, "Battery updated");
  
  // Register callbacks
  app_message_register_inbox_received(inbox_received_handler);
  
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
