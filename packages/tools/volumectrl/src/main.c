
#include <stdbool.h>
#include <stdio.h>
#include <signal.h>
#include <alsa/asoundlib.h>
#include <alsa/mixer.h>
#include <wiringPi.h>

static bool exit_flag = false;

static snd_mixer_t* mixer_handle = NULL;
static snd_mixer_elem_t* mixer_elem = NULL;

static const int VOLUME_MIN = 0;
static const int VOLUME_MAX = 100;
static const int VOLUME_STEP = 5;

bool mixer_init(void)
{
    if (snd_mixer_open(&mixer_handle, 0) < 0) {
        fprintf(stderr, "Failed to open mixer!\n");
        return false;
    }

    const char* card = "default";
    if (snd_mixer_attach(mixer_handle, card) < 0) {
        fprintf(stderr, "Failed to attach to mixer!\n");
        goto error1;
    }

    if (snd_mixer_selem_register(mixer_handle, NULL, NULL) < 0) {
        fprintf(stderr, "Failed to select register!\n");
        goto error1;
    }

    if (snd_mixer_load(mixer_handle) < 0) {
        fprintf(stderr, "Failed to load mixer!\n");
        goto error1;
    }

    snd_mixer_selem_id_t* sid = NULL;
    if (snd_mixer_selem_id_malloc(&sid) < 0) {
        fprintf(stderr, "Failed to allocate sid!\n");
        goto error2;
    }

    snd_mixer_selem_id_set_index(sid, 0);
    const char* selem_name = "PCM";
    snd_mixer_selem_id_set_name(sid, selem_name);
    mixer_elem = snd_mixer_find_selem(mixer_handle, sid);
    if (!mixer_elem) {
        fprintf(stderr, "Failed to find selem!\n");
        goto error2;
    }
    snd_mixer_selem_id_free(sid);

    if (snd_mixer_selem_set_playback_volume_range(mixer_elem, VOLUME_MIN, VOLUME_MAX) < 0) {
        fprintf(stderr, "Failed to set volume range!");
        goto error2;
    }

    return true;

error2:
    snd_mixer_free(mixer_handle);
error1:    
    (void)snd_mixer_close(mixer_handle);
    return false;
}

int mixer_read(void)
{
    long value;
    (void)snd_mixer_selem_get_playback_volume(mixer_elem, 0, &value);
    return (int)value;
}

void mixer_increase(void)
{
    int volume = mixer_read();
    volume += VOLUME_STEP;
    if (volume > VOLUME_MAX) {
        volume = VOLUME_MAX;
    }
    (void)snd_mixer_selem_set_playback_volume(mixer_elem, 0, (long)volume);
}

void mixer_decrease(void)
{
    int volume = mixer_read();
    volume -= VOLUME_STEP;
    if (volume < VOLUME_MIN) {
        volume = VOLUME_MIN;
    }
    (void)snd_mixer_selem_set_playback_volume(mixer_elem, 0, (long)volume);
}

void mixer_free(void)
{
    snd_mixer_free(mixer_handle);
    (void)snd_mixer_close(mixer_handle);
}

void signal_handler(int sig)
{
    exit_flag = true;
}

int main(int argc, char** argv)
{
    signal(SIGTERM, signal_handler);
    signal(SIGQUIT, signal_handler);
    signal(SIGHUP, signal_handler);

    if (!mixer_init()) {
        return 1;
    }

    (void)wiringPiSetupGpio();
    const int VOLUMEUP_PIN = 23;
    const int VOLUMEDOWN_PIN = 24;
    pinMode(VOLUMEUP_PIN, INPUT);
    pullUpDnControl(VOLUMEUP_PIN, PUD_UP);
    pinMode(VOLUMEDOWN_PIN, INPUT);
    pullUpDnControl(VOLUMEDOWN_PIN, PUD_UP);

    while (!exit_flag) {
        if (digitalRead(VOLUMEUP_PIN) == LOW) {
            mixer_increase();
        }

        if (digitalRead(VOLUMEDOWN_PIN) == LOW) {
            mixer_decrease();
        }
        delay(200);
    }

    mixer_free();

    return 0;
}

