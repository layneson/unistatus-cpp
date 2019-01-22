#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cassert>
#include <cfloat>
#include <cmath>

#include <signal.h>

#include <toml.h>
#include <curl/curl.h>

#include "json.hpp"
#include "display.hpp"
#include "colors.hpp"

struct Config {
    char* weather_api_key;
    char* weather_coordinates;
	unsigned char brightness; // Between 0 and 255.
};

bool load_config(const char* filename, Config* c) {
    FILE* config_file = fopen(filename, "r");
    if (!config_file) {
        fprintf(stderr, "[!] Failed to open config file '%s'!\n", filename);
        return false;
    }

    char error_buffer[500];
    toml_table_t* config = toml_parse_file(config_file, error_buffer, sizeof(error_buffer));
    fclose(config_file);
    if (!config) {
        fprintf(stderr, "[!] Failed to parse config file: %s\n", error_buffer);
        return false;
    }

    const char* raw;

    raw = toml_raw_in(config, "weather_api_key");
    if (!raw) {
        fprintf(stderr, "[!] Weather API key not found!\n");
        return false;
    }
    toml_rtos(raw, &(c->weather_api_key));

    raw = toml_raw_in(config, "weather_coordinates");
    if (!raw) {
        fprintf(stderr, "[!] Weather coordinates not found!\n");
        return false;
    }
    toml_rtos(raw, &(c->weather_coordinates));

	raw = toml_raw_in(config, "brightness");
	if (!raw) {
		printf("> Did not find brightness, using default.\n");
		c->brightness = 128;
	} else {
		int64_t brightness_toml;
		if (toml_rtoi(raw, &brightness_toml) != 0) {
			fprintf(stderr, "[!] Expected number for brightness!\n");
			return false;
		}
		if (brightness_toml < 0 || brightness_toml > 255) {
			fprintf(stderr, "[!] Expected brightness to be in range [0, 255]!\n");
			return false;
		}
		c->brightness = (unsigned char) brightness_toml;
	}

    toml_free(config);

    return true;
}

struct JsonBuffer {
    size_t size;
    char* data;
};

size_t write_callback(char* ptr, size_t size, size_t nmemb, void* userdata) {
    JsonBuffer* json_buffer = (JsonBuffer*) userdata;

    memcpy(json_buffer->data + json_buffer->size, ptr, size * nmemb);
    json_buffer->size += size * nmemb;

    return size * nmemb;
}

struct HourlyWeather {
    int hour; // Offset, so current hour is 0, next is 1, etc.

    float precip_probability;
    float temperature;
};


bool fetch_weather(Config* config, HourlyWeather* hourly_weather, int num_hours) {
    CURL* curl_session = curl_easy_init();
    if (!curl_session) {
        fprintf(stderr, "[!] Failed to initialize CURL!\n");
        return false;
    }

    char url_buffer[500];
    sprintf(url_buffer, "https://api.darksky.net/forecast/%s/%s?exclude=currently,minutely,daily,alerts,flags", config->weather_api_key, config->weather_coordinates);
    curl_easy_setopt(curl_session, CURLOPT_URL, url_buffer);

    JsonBuffer json_buffer = { 0, new char[1024 * 1024] };
    curl_easy_setopt(curl_session, CURLOPT_WRITEDATA, &json_buffer);

    curl_easy_setopt(curl_session, CURLOPT_WRITEFUNCTION, write_callback);

    CURLcode error;
    if ((error = curl_easy_perform(curl_session)) != CURLE_OK) {
        fprintf(stderr, "[!] Failed to make weather API request: %d\n", error);
        return false;
    }

    // Parse JSON.

    json::Context ctx;
    json::parse(&ctx, json_buffer.data, json_buffer.size);

    // Make sure we have a global object, not an array.
    if (ctx.tokens[0].type != JSMN_OBJECT) {
        fprintf(stderr, "[!] Expected a global object in weather data!\n");
        return false;
    }

    int hourly_idx = json::object_find(&ctx, 0, "hourly");
    if (hourly_idx == -1) {
        fprintf(stderr, "[!] Failed to find hourly weather data!\n");
        return false;
    }

    int hourly_data_idx = json::object_find(&ctx, hourly_idx, "data");
    if (hourly_data_idx == -1) {
        fprintf(stderr, "[!] Failed to find data within hourly weather data!\n");
        return false;
    }

    assert(ctx.tokens[hourly_data_idx].type == JSMN_ARRAY);

    // Make sure we don't overrun our hour array.
    if (num_hours > ctx.tokens[hourly_data_idx].size) {
        num_hours = ctx.tokens[hourly_data_idx].size;
    }

    {
        int hour_object_idx = hourly_data_idx + 1; // Advance past array.
        for (int hour = 0; hour < num_hours; hour++) {
            assert(ctx.tokens[hour_object_idx].type == JSMN_OBJECT);

            hourly_weather[hour].hour = hour;

            int precip_probability_idx = json::object_find(&ctx, hour_object_idx, "precipProbability");
            if (precip_probability_idx == -1) {
                fprintf(stderr, "[!] Failed to find precipitation probability for an hour!\n");
                return false;
            }

            hourly_weather[hour].precip_probability = json::parse_float(&ctx, precip_probability_idx);
            printf("> Precip Prob: %f\n", hourly_weather[hour].precip_probability);

            int temperature_idx = json::object_find(&ctx, hour_object_idx, "temperature");
            if (temperature_idx == -1) {
                fprintf(stderr, "[!] Failed to find temperature for an hour!\n");
                return false;
            }

            hourly_weather[hour].temperature = json::parse_float(&ctx, temperature_idx);
            printf("> Temperature: %f\n", hourly_weather[hour].temperature);

            hour_object_idx += json::recursive_size(&ctx, hour_object_idx);
        }
    }

    return true;
}

void render_weather(Config* config, HourlyWeather weather[15]) {
    // float min_temp = FLT_MAX;
    // float max_temp = FLT_MIN;

    // for (int i = 0; i < 15; i++) {
    //     if (weather[i].temperature < min_temp) {
    //         min_temp = weather[i].temperature;
    //     }

    //     if (weather[i].temperature > max_temp) {
    //         max_temp = weather[i].temperature;
    //     }
    // }

    // printf("For next 15 hours... low: %.2fF, high: %.2fF\n", min_temp, max_temp);

    const colors::rgb hot = { 1, 0, 0 };
    const colors::rgb cold = { 0, 0, 1 };

    const colors::hsv hot_hsv = colors::rgb2hsv(hot);
    const colors::hsv cold_hsv = colors::rgb2hsv(cold);

    const int hot_temp = 120;
    const int cold_temp = -10;

    for (int i = 0; i < 8; i++) {
        float temp = weather[i].temperature;

        float h = hot_hsv.h * (1 - (temp - hot_temp))/(cold_temp - hot_temp) + cold_hsv.h * (temp - hot_temp)/(cold_temp - hot_temp);

        colors::hsv temp_hsv = { h, 1, 1 };
        colors::rgb temp_rgb = colors::hsv2rgb(temp_hsv);

        display::set_pixel(i, 7, 255*temp_rgb.r, 255*temp_rgb.g, 255*temp_rgb.b);
    }

    for (int i = 0; i < 8; i++) {
        float perc = weather[i].precip_probability;

        int num_lit = (int)roundf(perc * 7.0f);

        for (int j = 0; j < num_lit; j++) {
            display::set_pixel(i, 6 - j, 0, 0, 250);
        }
    }
}

static void ctrl_c_handler(int signum) {
	display::quit();

	exit(0);
}

static void setup_handlers() {
    struct sigaction sa = {
        ctrl_c_handler
    };

    sigaction(SIGINT, &sa, NULL);
    sigaction(SIGTERM, &sa, NULL);
}

int main() {
    Config config;
    if (!load_config("config.toml", &config)) return 1;
    printf("> Weather API key: %s\n", config.weather_api_key);
    printf("> Weather coordinates: %s\n", config.weather_coordinates);
	printf("> Brightness: %u\n", config.brightness);

    HourlyWeather hourly_weather_15[15] = { 0 };
    if (!fetch_weather(&config, hourly_weather_15, 15)) return 1;

    display::init();

	display::set_brightness(config.brightness);

	setup_handlers();

    render_weather(&config, hourly_weather_15);

    while (true) {
        display::update();
    }

    return 0;
}
