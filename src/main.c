#include <pebble.h>

Window *window;
TextLayer *text_layer, *bt_layer, *brand_layer, *batt_layer, *date_layer;
char buffer[] = "00:00", batt_pc[] = "000%", date_buffer[] = "00";
GBitmap *face_bitmap;
BitmapLayer *face_layer;
Layer *minutes_layer, *hours_layer, *centre_layer;
static GPath *hour_path = NULL, *minute_path = NULL;

static const GPathInfo HOUR_PATH_INFO = {
	.num_points = 7,
	.points = (GPoint[]) {{-3,15},{-5,13},{-5,-35},{0,-40},{5,-35},{5,13},{3,15}}
};

static const GPathInfo MINUTE_PATH_INFO = {
	.num_points = 7,
	.points = (GPoint[]) {{-2,15},{-4,13},{-4,-61},{0,-65},{4,-61},{4,13},{2,15}}
};

void hours_layer_update_proc(Layer *hours_layer, GContext*ctx){
	//fill path
	graphics_context_set_fill_color(ctx, GColorWhite);
	gpath_draw_filled(ctx, hour_path);
	//stroke path
	graphics_context_set_stroke_color(ctx, GColorBlack);
	gpath_draw_outline(ctx, hour_path);
}

void minutes_layer_update_proc(Layer *hands_layer, GContext* ctx){
	//fill path
	graphics_context_set_fill_color(ctx, GColorWhite);
	gpath_draw_filled(ctx, minute_path);
	//stroke path
	graphics_context_set_stroke_color(ctx, GColorBlack);
	gpath_draw_outline(ctx, minute_path);
}

void centre_layer_update_proc(Layer *centre_layer, GContext* ctx){
	//fill circle
	graphics_context_set_fill_color(ctx, GColorWhite);
	graphics_fill_circle(ctx, (GPoint){72, 84}, 7);
	//stroke circle
	graphics_context_set_stroke_color(ctx, GColorBlack);
	graphics_draw_circle(ctx, (GPoint){72, 84}, 7);
}

void tick_handler(struct tm *tick_time, TimeUnits units_changed){
	//update digital clock
	strftime(buffer, sizeof("00:00"), "%H:%M", tick_time);
	strftime(date_buffer, sizeof("00"), "%d", tick_time);
	text_layer_set_text(text_layer, buffer);
	text_layer_set_text(date_layer, date_buffer);

	//update analog clock
	gpath_rotate_to(hour_path, (TRIG_MAX_ANGLE/360)*(((tick_time->tm_hour)*30)+((tick_time->tm_min)*0.5)));
	gpath_move_to(hour_path, (GPoint){72, 84});
	gpath_rotate_to(minute_path, (TRIG_MAX_ANGLE/360)*((tick_time->tm_min)*6));
	gpath_move_to(minute_path, (GPoint){72, 84});
}

void bt_handler(){
	if(bluetooth_connection_service_peek()){
		text_layer_set_text(bt_layer, "");
		//vibes_short_pulse();
	}
	else{
		text_layer_set_text(bt_layer, "No Bluetooth Connection");
		//vibes_short_pulse();
	}
}

void batt_handler(BatteryChargeState charge_state){
	//charge_status = "%";
	snprintf(batt_pc, sizeof("000"), "%d", charge_state.charge_percent);
	text_layer_set_text(batt_layer, batt_pc);
}

void window_load(Window *window){
	
	//load bitmap for background/watchface
	face_bitmap = gbitmap_create_with_resource(RESOURCE_ID_WATCHFACE);
	//add bitmap layer
	face_layer = bitmap_layer_create(GRect(0, 0, 144, 168));
	bitmap_layer_set_bitmap(face_layer, face_bitmap);
	layer_add_child(window_get_root_layer(window), bitmap_layer_get_layer(face_layer));
	
	//create text layer for bluetooth status
	bt_layer = text_layer_create(GRect(0, 130, 144, 38));
	text_layer_set_background_color(bt_layer, GColorClear);
	text_layer_set_text_color(bt_layer, GColorWhite);
	text_layer_set_text_alignment(bt_layer, GTextAlignmentCenter);
	text_layer_set_font(bt_layer, fonts_get_system_font(FONT_KEY_GOTHIC_14));
	layer_add_child(window_get_root_layer(window), (Layer*) bt_layer);
	
	//manually invoke bluetooth handler to refresh status on load
	bt_handler();
	
	//create text layer for 'pebble' brand at 6oclock
	brand_layer = text_layer_create(GRect(0, 145, 144, 23));
	text_layer_set_background_color(brand_layer, GColorClear);
	text_layer_set_text_color(brand_layer, GColorWhite);
	text_layer_set_text_alignment(brand_layer, GTextAlignmentCenter);
	text_layer_set_font(brand_layer, fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD));
	text_layer_set_text(brand_layer, "pebble");
	layer_add_child(window_get_root_layer(window), (Layer*) brand_layer);
	
	//create text layer for pebble battery status
	batt_layer = text_layer_create(GRect(5, 76, 20, 50));
	text_layer_set_background_color(batt_layer, GColorClear);
	text_layer_set_text_color(batt_layer, GColorWhite);
	text_layer_set_text_alignment(batt_layer, GTextAlignmentCenter);
	text_layer_set_font(batt_layer, fonts_get_system_font(FONT_KEY_GOTHIC_14_BOLD));
	layer_add_child(window_get_root_layer(window), (Layer*) batt_layer);
	
	//manually invoke battery status handler to refresh status on load
	batt_handler(battery_state_service_peek());
	
	//add text layer for digital time
	text_layer = text_layer_create(GRect(0, 0, 144, 168));
	text_layer_set_background_color(text_layer, GColorClear);
	text_layer_set_text_color(text_layer, GColorWhite);
	text_layer_set_text_alignment(text_layer, GTextAlignmentCenter);
	text_layer_set_font(text_layer, fonts_get_system_font(FONT_KEY_GOTHIC_14_BOLD));
	layer_add_child(window_get_root_layer(window), (Layer*) text_layer);
	
	//create text layer for date
	date_layer = text_layer_create(GRect(116, 76, 25, 50));
	text_layer_set_background_color(date_layer, GColorClear);
	text_layer_set_text_color(date_layer, GColorWhite);
	text_layer_set_text_alignment(date_layer, GTextAlignmentCenter);
	text_layer_set_font(date_layer, fonts_get_system_font(FONT_KEY_GOTHIC_14_BOLD));
	layer_add_child(window_get_root_layer(window), (Layer*) date_layer);
	
	//add layers for analog clock
	hours_layer = layer_create(GRect(0, 0, 144, 168));
	layer_set_update_proc(hours_layer, hours_layer_update_proc);
	minutes_layer = layer_create(GRect(0, 0, 144, 168));
	layer_set_update_proc(minutes_layer, minutes_layer_update_proc);
	//draw hands
	hour_path = gpath_create(&HOUR_PATH_INFO);
	layer_add_child(window_get_root_layer(window), (Layer*) hours_layer);
	minute_path = gpath_create(&MINUTE_PATH_INFO);
	layer_add_child(window_get_root_layer(window), (Layer*) minutes_layer);
	
	//time struct to display time on load
	struct tm *t;
	time_t temp;
	temp = time(NULL);
	t = localtime(&temp);
	
	//manually invoke tick handler to refresh time on load
	tick_handler(t, MINUTE_UNIT);
	
	//add centre point
	centre_layer = layer_create(GRect(0, 0, 144, 168));
	layer_set_update_proc(centre_layer, centre_layer_update_proc);
	layer_add_child(window_get_root_layer(window), (Layer*) centre_layer);
}
 
void window_unload(Window *window){
    text_layer_destroy(text_layer);
	text_layer_destroy(bt_layer);
	text_layer_destroy(brand_layer);
	text_layer_destroy(batt_layer);
	text_layer_destroy(date_layer);
	gbitmap_destroy(face_bitmap);
	bitmap_layer_destroy(face_layer);
	layer_destroy(minutes_layer);
	layer_destroy(hours_layer);
	layer_destroy(centre_layer);
}

void handle_init(void){
	window = window_create();
	window_set_window_handlers(window, (WindowHandlers) {
        .load = window_load,
    	.unload = window_unload,
    });
	window_stack_push(window, true);
	tick_timer_service_subscribe(MINUTE_UNIT, (TickHandler) tick_handler);
	bluetooth_connection_service_subscribe((BluetoothConnectionHandler) bt_handler);
	battery_state_service_subscribe((BatteryStateHandler) batt_handler);
}

void handle_deinit(void){
	window_destroy(window);
	tick_timer_service_unsubscribe();
	bluetooth_connection_service_unsubscribe();
	battery_state_service_unsubscribe();
}

int main(void){
	  handle_init();
	  app_event_loop();
	  handle_deinit();
}