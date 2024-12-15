/**
 * @file metronome.h
 * @brief Metronome logic
 * @author Milan Vodák <xvodak07@stud.fit.vut.cz>
 */

#define DEFAULT_BPM 120
#define DEFAULT_VOLUME 50
#define DEFAULT_BEATS 4

#define PWM_SPEED_MODE LEDC_HIGH_SPEED_MODE
#define PWM_CHANNEL LEDC_CHANNEL_0
#define PWM_TIMER LEDC_TIMER_0
#define PWM_FREQUENCY 3500
#define PWM_DUTY_RES LEDC_TIMER_13_BIT
#define PWM_MAX_DUTY 6144 // 75 % of 2 ** 13

void metronome_init();
void metronome_loop();
void metronome_start();
void metronome_stop();
void metronome_set_bpm(unsigned int bpm);
void metronome_set_volume(unsigned int volume);
void metronome_set_beats(unsigned int beats);
char* metronome_status_json();
