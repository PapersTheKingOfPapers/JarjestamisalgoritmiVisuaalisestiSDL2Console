#include <iostream>
#include <SDL.h>
#include <vector>
#include <random>
#include <algorithm>
#include <thread>
#include <mutex>

using namespace std;

const int SCREEN_WIDTH = 800;
const int SCREEN_HEIGHT = 600;
const int BAR_WIDTH = 4;
const int ELEMENT_COUNT = SCREEN_WIDTH / BAR_WIDTH;

vector<int> data_vector(ELEMENT_COUNT);
int active_idx1 = -1;
int active_idx2 = -1;
bool is_running = true;

mutex data_mutex;

void render(SDL_Renderer* renderer) {
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);

    lock_guard<mutex> lock(data_mutex);

    for (int i = 0; i < data_vector.size(); i++) {
        int bar_height = data_vector[i];

        if (i == active_idx1 || i == active_idx2) SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
        else SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);

        SDL_Rect rect;
        rect.x = i * BAR_WIDTH;
        rect.y = SCREEN_HEIGHT - bar_height;
        rect.w = BAR_WIDTH - 1;
        rect.h = bar_height;

        SDL_RenderFillRect(renderer, &rect);
    }

    SDL_RenderPresent(renderer);
}

void bubbleSort() {
    int n = data_vector.size();

    for (int i = 0; i < n - 1; i++) {
        bool swapped = false;
        for (int j = 0; j < n - i - 1; j++) {
            if (!is_running) return;
            {
                lock_guard<mutex> lock(data_mutex);
                if (data_vector[j] > data_vector[j + 1]) {
                    swap(data_vector[j], data_vector[j + 1]);
                    swapped = true;
                }
                active_idx1 = j;
                active_idx2 = j + 1;
            }
            this_thread::sleep_for(chrono::microseconds(50));
        }
        if (!swapped) break;
    }
    lock_guard<mutex> lock(data_mutex);
    active_idx1 = -1;
    active_idx2 = -1;
}

int main(int argc, char* args[])
{
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        cerr << "SDL alustus epäonnistui: " << SDL_GetError() << endl;
        return -1;
    }

    SDL_Window* window = SDL_CreateWindow("Saikeistetty Jarjestaminen SDL2:n avulla",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

    for (int i = 0; i < ELEMENT_COUNT; i++) {
        data_vector[i] = 10 + (i * (SCREEN_HEIGHT - 20) / ELEMENT_COUNT);
    }

    random_device rd;
    unsigned int seed = rd(); // Tuli error " 'generate': is not a member of 'std::random_device' " ilman tätä.
    mt19937 g(seed);
    shuffle(data_vector.begin(), data_vector.end(), g);

    thread sorting_thread(bubbleSort); // Järjestysalgoritmi tapahtuu säikeistettynä, jotta renderaus olisi tasaista ja pehmeää.

    SDL_Event event;
    while (is_running) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) { // Suoritetaan, kun ruutu suljetaan ruksista.
                is_running = false;
            }
        }
        render(renderer); //Renderaa ruudun
        SDL_Delay(16);
    }

    if (sorting_thread.joinable()) sorting_thread.join();

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}
