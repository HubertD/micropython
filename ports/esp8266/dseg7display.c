#include "py/runtime.h"
#include "modmachine.h"
#include "espneopixel.h"

#define NUM_DIGITS 2
#define NUM_SEGMENTS 7
#define NUM_PIXELS 141
#define BYTES_PER_PIXEL 3

static const char* _segments[NUM_DIGITS][NUM_SEGMENTS] =
{
	{
		"\x24\x23\x22\x21\x20\x1F\x11\x10\x0F\x0E\x0D",
		"\x16\x17\x18\x19\x1A\x3F\x40\x41\x42\x43",
		"\x31\x32\x33\x34\x35\x36\x37\x38\x39\x3A",
		"\x25\x26\x27\x28\x29\x2A\x12\x13\x14\x15",
		"\x0C\x0B\x0A\x09\x08\x07\x1E\x1D\x1C\x1B",
		"\x2B\x2C\x2D\x2E\x2F\x30\x3B\x3C\x3D\x3E",
		"\x01\x02\x03\x04\x05\x06\x44\x45\x46\x47"
	},
	{
		"\x61\x62\x63\x64\x65\x66\x67\x68\x69\x6A",
		"\x58\x59\x5A\x5B\x5C\x85\x86\x87\x88",
		"\x77\x78\x79\x7A\x7B\x7C\x7D\x7E\x7F\x80",
		"\x6B\x6C\x6D\x6E\x6F\x70\x5D\x5E\x5F\x60",
		"\x54\x55\x56\x57\x4E\x4F\x50\x51\x52\x53",
		"\x71\x72\x73\x74\x75\x76\x81\x82\x83\x84",
		"\x48\x49\x4A\x4B\x4C\x4D\x89\x8A\x8B\x8C"
	}
};

static const uint8_t _font[] = 
{
	0b1011111,  // 0
	0b0000101,  // 1
	0b1110110,  // 2
	0b1110101,  // 3
	0b0101101,  // 4
	0b1111001,  // 5
	0b1111011,  // 6
	0b1000101,  // 7
	0b1111111,  // 8
	0b1111101,  // 9
}; // HHHVVVV
   // 1231234


/* object data members */
typedef struct {
    mp_obj_base_t base;
    pyb_pin_obj_t* pin;
    uint8_t buffer[NUM_PIXELS*BYTES_PER_PIXEL];
} Dseg7Display_obj_t;


/* repr... */
STATIC void Dseg7Display_print(const mp_print_t *print, mp_obj_t self_in, mp_print_kind_t kind) 
{
}


/* Dseg7Display::write(self) */
STATIC mp_obj_t Dseg7Display_write(mp_obj_t self_in) 
{
    Dseg7Display_obj_t *self = MP_OBJ_TO_PTR(self_in);
	esp_neopixel_write(self->pin->phys_port, self->buffer, sizeof(self->buffer), true);
    return mp_const_none;
}
MP_DEFINE_CONST_FUN_OBJ_1(Dseg7Display_write_obj, Dseg7Display_write);

static bool dseg7display_set_led(Dseg7Display_obj_t *self, int led_num, uint8_t r, uint8_t g, uint8_t b)
{
    if ((led_num<0) || (led_num>=NUM_PIXELS)) { return false; }
    int idx = BYTES_PER_PIXEL * led_num;
    self->buffer[idx+0] = g;
    self->buffer[idx+1] = r;
    self->buffer[idx+2] = b;
    return true;
}

static bool dseg7display_set_segment(Dseg7Display_obj_t *self, uint8_t digit, uint8_t segment, uint8_t r, uint8_t g, uint8_t b)
{
	if (digit>=NUM_DIGITS) { return false; }
	if (segment>=NUM_SEGMENTS) { return false; }

	const char *segdata = _segments[digit][segment];
	for (size_t i=0; segdata[i]!=0; i++)
	{
		dseg7display_set_led(self, segdata[i], r, g, b);
	}

    return true;
}

static bool dseg7display_set_number(Dseg7Display_obj_t *self, uint8_t digit, uint8_t number, uint8_t r, uint8_t g, uint8_t b)
{
	if (digit>=NUM_DIGITS) { return false; }
	if (number>9) { return false; }
	uint8_t data = _font[number];
	
	for (unsigned i=0; i<7; i++)
	{
		if (data & 0x40)
		{
			dseg7display_set_segment(self, digit, i, r, g, b);
		}
		else
		{
			dseg7display_set_segment(self, digit, i, 0, 0, 0);
		}
		data <<= 1;		
	}
    return true;
}

/* Dseg7Display::set_led(self, led_num, r, g, b) */
STATIC mp_obj_t Dseg7Display_set_led(size_t n_args, const mp_obj_t *args)
{
    (void)n_args; // always 5
    dseg7display_set_led(MP_OBJ_TO_PTR(args[0]), mp_obj_get_int(args[1]), mp_obj_get_int(args[2]), mp_obj_get_int(args[3]), mp_obj_get_int(args[4]));
    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(Dseg7Display_set_led_obj, 5, 5, Dseg7Display_set_led);


/* Dseg7Display::set_segment(self, digit, segment, r, g, b) */
STATIC mp_obj_t Dseg7Display_set_segment(size_t n_args, const mp_obj_t *args)
{
    (void)n_args; // always 6
    dseg7display_set_segment(MP_OBJ_TO_PTR(args[0]), mp_obj_get_int(args[1]), mp_obj_get_int(args[2]), mp_obj_get_int(args[3]), mp_obj_get_int(args[4]), mp_obj_get_int(args[5]));
    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(Dseg7Display_set_segment_obj, 6, 6, Dseg7Display_set_segment);


/* Dseg7Display::set_digit(self, digit, number, r, g, b) */
STATIC mp_obj_t Dseg7Display_set_digit(size_t n_args, const mp_obj_t *args)
{
    (void)n_args; // always 6
    dseg7display_set_number(MP_OBJ_TO_PTR(args[0]), mp_obj_get_int(args[1]), mp_obj_get_int(args[2]), mp_obj_get_int(args[3]), mp_obj_get_int(args[4]), mp_obj_get_int(args[5]));
    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(Dseg7Display_set_digit_obj, 6, 6, Dseg7Display_set_digit);


/* Dseg7Display::fill(self, r, g, b) */
STATIC mp_obj_t Dseg7Display_fill(size_t n_args, const mp_obj_t *args)
{
    (void)n_args; // always 4
	for (size_t i=0; i<NUM_PIXELS; i++)
	{
		dseg7display_set_led(MP_OBJ_TO_PTR(args[0]), i, mp_obj_get_int(args[1]), mp_obj_get_int(args[2]), mp_obj_get_int(args[3]));
	}
    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(Dseg7Display_fill_obj, 4, 4, Dseg7Display_fill);


/* Dseg7Display::set_number(self, number, r, g, b) */
STATIC mp_obj_t Dseg7Display_set_number(size_t n_args, const mp_obj_t *args)
{
    (void)n_args; // always 5
    
    int number = mp_obj_get_int(args[1]);
    if (number<0) { number = 0; }
    if (number>99) { number = 99; }
    
    uint8_t r = mp_obj_get_int(args[2]);
    uint8_t g = mp_obj_get_int(args[3]);
    uint8_t b = mp_obj_get_int(args[4]);
    
    dseg7display_set_number(MP_OBJ_TO_PTR(args[0]), 0, (number%10), r, g, b);
    dseg7display_set_number(MP_OBJ_TO_PTR(args[0]), 1, ((number/10)%10), r, g, b);

    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(Dseg7Display_set_number_obj, 5, 5, Dseg7Display_set_number);


STATIC const mp_rom_map_elem_t Dseg7Display_locals_dict_table[] = 
{
	{ MP_ROM_QSTR(MP_QSTR_write), MP_ROM_PTR(&Dseg7Display_write_obj) },
	{ MP_ROM_QSTR(MP_QSTR_set_led), MP_ROM_PTR(&Dseg7Display_set_led_obj) },
	{ MP_ROM_QSTR(MP_QSTR_set_segment), MP_ROM_PTR(&Dseg7Display_set_segment_obj) },
	{ MP_ROM_QSTR(MP_QSTR_set_digit), MP_ROM_PTR(&Dseg7Display_set_digit_obj) },
	{ MP_ROM_QSTR(MP_QSTR_fill), MP_ROM_PTR(&Dseg7Display_fill_obj) },
	{ MP_ROM_QSTR(MP_QSTR_set_number), MP_ROM_PTR(&Dseg7Display_set_number_obj) },
};
STATIC MP_DEFINE_CONST_DICT(Dseg7Display_locals_dict, Dseg7Display_locals_dict_table);


/* constructor declaration */
mp_obj_t Dseg7Display_make_new(const mp_obj_type_t *type, size_t n_args, size_t n_kw, const mp_obj_t *args);

const mp_obj_type_t Dseg7Display_type = 
{
    { &mp_type_type },
    .name = MP_QSTR_Dseg7Display,
    .print = Dseg7Display_print,
    .make_new = Dseg7Display_make_new,
    .locals_dict = (mp_obj_dict_t*)&Dseg7Display_locals_dict,
};


/* constructor definition */
mp_obj_t Dseg7Display_make_new(const mp_obj_type_t *type, size_t n_args, size_t n_kw, const mp_obj_t *args)
{
    mp_arg_check_num(n_args, n_kw, 1, 1, true);
    Dseg7Display_obj_t *self = m_new_obj(Dseg7Display_obj_t);
    self->base.type = &Dseg7Display_type;
    self->pin = mp_obj_get_pin_obj(args[0]);
    return MP_OBJ_FROM_PTR(self);
}


STATIC const mp_map_elem_t dseg7display_globals_table[] = 
{
    { MP_OBJ_NEW_QSTR(MP_QSTR___name__), MP_OBJ_NEW_QSTR(MP_QSTR_dseg7display) },
    { MP_OBJ_NEW_QSTR(MP_QSTR_Dseg7Display), (mp_obj_t)&Dseg7Display_type },
};
STATIC MP_DEFINE_CONST_DICT(mp_module_dseg7display_globals, dseg7display_globals_table);

const mp_obj_module_t mp_module_dseg7display = {
    .base = { &mp_type_module },
    .globals = (mp_obj_dict_t*)&mp_module_dseg7display_globals,
};
