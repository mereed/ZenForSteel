#include <pebble.h>

//WINDOW
Window* window;
//LAYER
Layer* layer;
//FONT
GFont raleway_font;
//BACKGROUND
GBitmap* background;
//SETTINGS
bool seconds = true;
bool weekday = true;
bool date = false;
bool month = false;

static uint8_t batteryPercent;
static GBitmap *battery_image;
static BitmapLayer *battery_image_layer;
static BitmapLayer *battery_layer;

static GBitmap *bluetooth_image;
static BitmapLayer *bluetooth_layer;

void change_battery_icon(bool charging) {
  gbitmap_destroy(battery_image);
  if(charging) {
    battery_image = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_BATTERY_CHARGE);
  }
  else {
    battery_image = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_BATTERY);
  }  
  bitmap_layer_set_bitmap(battery_image_layer, battery_image);
  layer_mark_dirty(bitmap_layer_get_layer(battery_image_layer));
}

static void update_battery(BatteryChargeState charge_state) {

  batteryPercent = charge_state.charge_percent;

  if(batteryPercent==100) {
	change_battery_icon(false); 
    return;
  }

  layer_set_hidden(bitmap_layer_get_layer(battery_layer), charge_state.is_charging);
  change_battery_icon(charge_state.is_charging);
    
}

static void toggle_bluetooth_icon(bool connected) {
  if(!connected) {
    //vibe!
    vibes_long_pulse();
  }
  layer_set_hidden(bitmap_layer_get_layer(bluetooth_layer), !connected);
}

void bluetooth_connection_callback(bool connected) {
  toggle_bluetooth_icon(connected);
}

void battery_layer_update_callback(Layer *me, GContext* ctx) {        
  //draw the remaining battery percentage
  graphics_context_set_stroke_color(ctx, GColorWhite);
  graphics_context_set_fill_color(ctx, GColorWhite);
  graphics_fill_rect(ctx, GRect(2, 2, ((batteryPercent/100.0)*11.0), 5), 0, GCornerNone);
}
	
void update_layer(Layer *me, GContext* ctx) 
{
	//watchface drawing
	
	char text[10];
	
	graphics_context_set_stroke_color(ctx,GColorWhite);
	graphics_context_set_text_color(ctx, GColorWhite);
	
	//draw background
	graphics_draw_bitmap_in_rect(ctx,background,GRect(0,0,144,168));
	
	//get tick_time
	time_t temp = time(NULL); 
  	struct tm *tick_time = localtime(&temp);
	
	//get weekday
	strftime(text, 8, "%l:%M%P", tick_time);
	//lowercase
	//text[0] += 32;
	
	if(weekday == 1)
		graphics_draw_text(ctx, text,  raleway_font, GRect(0,-6,144,100), GTextOverflowModeTrailingEllipsis, GTextAlignmentCenter, NULL);
	
	//dumb friday fix
	if(text[0] == 'f'){
		graphics_draw_pixel(ctx, GPoint(49,0));
		graphics_draw_pixel(ctx, GPoint(50,0));
	}
	
	strftime(text, 10, "%d", tick_time);
	
	if(date == 1)
		graphics_draw_text(ctx, text, fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD), GRect(124,86,23,30), GTextOverflowModeTrailingEllipsis, GTextAlignmentCenter, NULL);
	
	strftime(text, 10, "%a", tick_time);
	//lowercase
	text[0] += 32;
	
	if(month == 1)
		graphics_draw_text(ctx, text,  fonts_get_system_font(FONT_KEY_GOTHIC_14), GRect(0,88,16,25), GTextOverflowModeTrailingEllipsis, GTextAlignmentCenter, NULL);
	
	//draw hands
	GPoint center = GPoint(71,99);
	int16_t secondHandLength = 66;
	int16_t minuteHandLength = 60;
	int16_t hourHandLength = 34;
	GPoint secondHand;
	GPoint minuteHand;
	GPoint hourHand;
	
	

	int32_t second_angle = TRIG_MAX_ANGLE * tick_time->tm_sec / 60;
	secondHand.y = (int16_t)(-cos_lookup(second_angle) * (int32_t)secondHandLength / TRIG_MAX_RATIO) + center.y;
	secondHand.x = (int16_t)(sin_lookup(second_angle) * (int32_t)secondHandLength / TRIG_MAX_RATIO) + center.x;
	
	if(seconds == 1)
		graphics_draw_line(ctx, center, secondHand);
	
	int32_t minute_angle = TRIG_MAX_ANGLE * tick_time->tm_min / 60;
	minuteHand.y = (int16_t)(-cos_lookup(minute_angle) * (int32_t)minuteHandLength / TRIG_MAX_RATIO) + center.y;
	minuteHand.x = (int16_t)(sin_lookup(minute_angle) * (int32_t)minuteHandLength / TRIG_MAX_RATIO) + center.x;
	graphics_draw_line(ctx, center, minuteHand);
	
	int32_t hour_angle = (TRIG_MAX_ANGLE * (((tick_time->tm_hour % 12) * 6) + (tick_time->tm_min / 10))) / (12 * 6);
	hourHand.y = (int16_t)(-cos_lookup(hour_angle) * (int32_t)hourHandLength / TRIG_MAX_RATIO) + center.y;
	hourHand.x = (int16_t)(sin_lookup(hour_angle) * (int32_t)hourHandLength / TRIG_MAX_RATIO) + center.x;
	graphics_draw_line(ctx, center, hourHand);
	
	
	//I didn't like how a 2px path rotated, so I'm using two lines next to each other
	//I need to move the pixels from vertically adjacent to horizontally adjacent based on the position
	bool addX = (tick_time->tm_min > 20 && tick_time->tm_min < 40) || tick_time->tm_min < 10 || tick_time->tm_min > 50;
	center.x+=addX?1:0;
	center.y+=!addX?1:0;
	minuteHand.x+=addX?1:0;
	minuteHand.y+=!addX?1:0;
	graphics_draw_line(ctx, center, minuteHand);
	
	center.x-=addX?1:0;
	center.y-=!addX?1:0;
	
	addX = (tick_time->tm_hour >= 4 && tick_time->tm_hour <= 8) || tick_time->tm_hour < 2 || tick_time->tm_hour > 10;
	center.x+=addX?1:0;
	center.y+=!addX?1:0;
	hourHand.x+=addX?1:0;
	hourHand.y+=!addX?1:0;
	graphics_draw_line(ctx, center, hourHand);

}

void tick(struct tm *tick_time, TimeUnits units_changed)
{
	//redraw every tick
	layer_mark_dirty(layer);
}

static void inbox_received_callback(DictionaryIterator *iterator, void *context) {
	
	APP_LOG(APP_LOG_LEVEL_INFO, "inbox");
	
	//seconds
	Tuple *t = dict_find(iterator, 0);
	seconds = (int)t->value->int32 == 1;
		
	tick_timer_service_unsubscribe();	
	tick_timer_service_subscribe(seconds?SECOND_UNIT:MINUTE_UNIT, (TickHandler) tick);
	
	//weekday
	t = dict_find(iterator, 1);
	weekday = (int)t->value->int32 == 1;
	
	//date
	t = dict_find(iterator, 2);
	date = (int)t->value->int32 == 1;
	
	//month
	t = dict_find(iterator, 3);
	month = (int)t->value->int32 == 1;
	
	//redraw
	layer_mark_dirty(layer);
		
}

static void inbox_dropped_callback(AppMessageResult reason, void *context) {}

static void outbox_failed_callback(DictionaryIterator *iterator, AppMessageResult reason, void *context) {}

static void outbox_sent_callback(DictionaryIterator *iterator, void *context) {}


void init() 
{
	//create window
	window = window_create();
	window_set_background_color(window,GColorBlack);
	window_stack_push(window, true);
	Layer* window_layer = window_get_root_layer(window);	
	
	//load background
	background = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_BACKGROUND);
	
	//load font
	raleway_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_RALEWAY_22));
	
	//create layer
	layer = layer_create(GRect(0,0,144,168));
	layer_set_update_proc(layer, update_layer);
	layer_add_child(window_layer, layer);	
	
    bluetooth_image = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_BLUETOOTH);
    GRect frame3 = (GRect) {
    .origin = { .x = 67, .y = 157 },
    .size = bluetooth_image->bounds.size
    };
    bluetooth_layer = bitmap_layer_create(frame3);
    bitmap_layer_set_bitmap(bluetooth_layer, bluetooth_image);
    layer_add_child(window_layer, bitmap_layer_get_layer(bluetooth_layer));
	
    battery_image = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_BATTERY);
    GRect frame4 = (GRect) {
    .origin = { .x = 63, .y = 27 },
    .size = battery_image->bounds.size
    };
    battery_layer = bitmap_layer_create(frame4);
    battery_image_layer = bitmap_layer_create(frame4);
    bitmap_layer_set_bitmap(battery_image_layer, battery_image);
    layer_set_update_proc(bitmap_layer_get_layer(battery_layer), battery_layer_update_callback);
  
    layer_add_child(window_layer, bitmap_layer_get_layer(battery_image_layer));
    layer_add_child(window_layer, bitmap_layer_get_layer(battery_layer));
  
	toggle_bluetooth_icon(bluetooth_connection_service_peek());
    update_battery(battery_state_service_peek());

	//subscribe to seconds tick event
	tick_timer_service_subscribe(SECOND_UNIT, (TickHandler) tick);
		
    bluetooth_connection_service_subscribe(bluetooth_connection_callback);
    battery_state_service_subscribe(&update_battery);

	// Register callbacks
	app_message_register_inbox_received(inbox_received_callback);
	app_message_register_inbox_dropped(inbox_dropped_callback);
	app_message_register_outbox_failed(outbox_failed_callback);
	app_message_register_outbox_sent(outbox_sent_callback);

	// Open AppMessage
	app_message_open(app_message_inbox_size_maximum(), app_message_outbox_size_maximum());
	
}

void deinit() 
{
	layer_destroy(layer);
	fonts_unload_custom_font(raleway_font);
	gbitmap_destroy(background);
	
	bluetooth_connection_service_unsubscribe();
    battery_state_service_unsubscribe();
	
	layer_remove_from_parent(bitmap_layer_get_layer(bluetooth_layer));
    bitmap_layer_destroy(bluetooth_layer);
    gbitmap_destroy(bluetooth_image);
    bluetooth_image = NULL;
	
    layer_remove_from_parent(bitmap_layer_get_layer(battery_layer));
    bitmap_layer_destroy(battery_layer);
    gbitmap_destroy(battery_image);
    battery_image = NULL;
	
    layer_remove_from_parent(bitmap_layer_get_layer(battery_image_layer));
    bitmap_layer_destroy(battery_image_layer);
	
	window_destroy(window);
}

int main()
{
	init();
	app_event_loop();
	deinit();
}
