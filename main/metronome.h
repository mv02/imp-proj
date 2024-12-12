/**
 * @file metronome.h
 * @brief Metronome logic
 * @author Milan Vodák <xvodak07@stud.fit.vut.cz>
 */

#define DEFAULT_BPM 120

void metronome_init();
void metronome_start();
void metronome_set_bpm(unsigned int bpm);
