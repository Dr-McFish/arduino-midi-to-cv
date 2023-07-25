#include "settings.h"

int modulo(int x,int N){
    return (x % N + N) %N;
}

void edit_settings(int encoder_dir, bool encoder_button, struct settings_s* settings) {
	if (encoder_button){
		if (setting_type[settings->selected_option] == NUMBER_ST)
			settings->is_editing = settings->is_editing != 1; /* XOR to flip the value */
		else if (setting_type[settings->selected_option] == ONOFF_ST || setting_type[settings->selected_option] == ONOFF_LBL_ST){
		settings->flags ^= _BV((settings->selected_option - SETTINGS_NUM_OF_NUMS)); // ^ XOR to flip the flag
		}
	}
	
	if(settings->is_editing){
		settings->nums[settings->selected_option] = 
					max(settings_num_ranges[settings->selected_option].min,
						min(settings_num_ranges[settings->selected_option].max, 
								settings->nums[settings->selected_option] + encoder_dir));
	} else {
		settings->selected_option = modulo(settings->selected_option + encoder_dir, NUM_OF_SETTINGS_LINES);
	}
}

void load_settings_from_EEPROM(struct settings_s* settings){}
void save_settings_to_EEPROM(const struct settings_s* settings){}
