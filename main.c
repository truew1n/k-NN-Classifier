#include <stdio.h>

#include "parser.h"
#include "hashmap.h"

#include <windows.h>

typedef struct dv_t {
    float dw;
    int32_t i;
} dv_t;

#define TYPE dv_t
#include "array.h"
#undef TYPE

typedef struct param_t {
    void *x;
    void *y;
} param_t;

int32_t dv_t_comparator(dv_t a, dv_t b);
char *classify(trainset_t *trainset, float_array_t values, int32_t k);

typedef struct point_t {
    int32_t x;
    int32_t y;
} point_t;

#define ABS(x) (x < 0 ? x * -1 : x)
float mapf(float number, float left_range, float right_range, float left_bound, float right_bound);

void gc_draw_pixel(void *display, point_t point, uint32_t color, point_t display_info);
void gc_fill_rect(void *display, point_t point, point_t dimension, uint32_t color, point_t display_info);
void gc_draw_line(void *display, point_t start, point_t end, uint32_t color, point_t display_info);

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
DWORD WINAPI VisualThread(LPVOID lpParam);


int main(void)
{
    /*  Lepiej, żeby pliki testowe były w jakimś folderze, a nie luźno leżące (po prostu czyściej jest).
        Od komentować w razie potrzeby
    */
    // trainset_t iris_trainset = parse_trainset_file("iris.data");
    // trainset_t iris_test_trainset = parse_trainset_file("iris.test.data");

    // trainset_t wdbc_trainset = parse_trainset_file("wdbc.data");
    // trainset_t wdbc_test_trainset = parse_trainset_file("wdbc.test.data");

    trainset_t iris_trainset = parse_trainset_file("iris/iris.data");
    trainset_t iris_test_trainset = parse_trainset_file("iris/iris.test.data");

    trainset_t wdbc_trainset = parse_trainset_file("wdbc/wdbc.data");
    trainset_t wdbc_test_trainset = parse_trainset_file("wdbc/wdbc.test.data");

    int current_trainset = 0;
    trainset_t trainset = iris_trainset;
    trainset_t test_trainset = iris_test_trainset;

    param_t visual_params;
    visual_params.x = &trainset;
    visual_params.y = &test_trainset;

    printf("See help command!!!!\n");

    char input_buffer[4096];
    int8_t running = 1;
    while(running) {
        printf("Write input vector for classification:\n");
        printf("(Allowed float notations: 1.0e+3 ; 13e-1 ; 1.0 ; 1)\n");
        printf("(Format: 0.23,0.32,0.45,6.4,23.4,23.3,...)\n");
        printf(">: ");
        scanf("%4096s", input_buffer);

        if(!strcmp(input_buffer, "exit")) {
            running = 0;
        } else if(!strcmp(input_buffer, "help")) {
            printf("\n");
            printf("Commands:\n");
            printf("\texit - exits the program\n");
            printf("\tclassify - classifies all test cases\n");
            printf("\tvisualize - visualizes accuracy of test cases\n");
            printf("\tswap - swaps between iris and wdbc trainsets\n");
            printf("\n");
        } else if(!strcmp(input_buffer, "swap")) {
            switch(current_trainset) {
                case 0: {
                    trainset = wdbc_trainset;
                    test_trainset = wdbc_test_trainset;
                    current_trainset++;
                    printf("\n");
                    printf("You are currently using wdbc trainset!!!\n");
                    printf("\n");
                    break;
                }
                case 1: {
                    trainset = iris_trainset;
                    test_trainset = iris_test_trainset;
                    current_trainset = 0;
                    printf("\n");
                    printf("You are currently using iris trainset!!!\n");
                    printf("\n");
                    break;
                }
            }
        } else if(!strcmp(input_buffer, "classify")) {
            printf("\n");
            for(int32_t k = 1; k < trainset.records.size; ++k) {
                int32_t count = 0;
                
                ARRAY_ITER(test_trainset.records, j) {
                    record_t record = array_get_record_t(&test_trainset.records, j);
                    char *classification = classify(&trainset, record.values, k);
                    if(!strcmp(classification, record.classname.data)) {
                        count++;
                    }
                }

                printf(
                    "k = %i, accuracy = %f%% (%i/%i)\n",
                    k,
                    ((float) count) / test_trainset.records.size,
                    count,
                    test_trainset.records.size
                );
            }

            printf("\n");
        } else if(!strcmp(input_buffer, "visualize")) {
            printf("\n");
            HANDLE hThread;
            hThread = CreateThread(NULL, 0, VisualThread, &visual_params, 0, NULL);
            if (hThread == NULL) {
                fprintf(stderr, "Error creating thread\n");
                return 1;
            }
            WaitForSingleObject(hThread, INFINITE);
            CloseHandle(hThread);
            printf("Visualization closed!!!\n");
            printf("\n");
        } else {
            float_array_t input_vector = parse_input_line(input_buffer);
            
            printf("\nInput Vector: [");
            for(int32_t i = 0; i < input_vector.size - 1; ++i) {
                printf("%f,", array_get_float(&input_vector, i));
            } printf("%f]\n", array_get_float(&input_vector, input_vector.size - 1));

            printf("Classification: %s\n\n", classify(&trainset, input_vector, 4));
        }
    }
    
    trainset_dealloc(&iris_trainset);
    trainset_dealloc(&iris_test_trainset);
    trainset_dealloc(&wdbc_trainset);
    trainset_dealloc(&wdbc_test_trainset);
    return 0;
}

int32_t dv_t_comparator(dv_t a, dv_t b)
{
    if(a.dw < b.dw) return -1;
    else if(a.dw > b.dw) return 1;
    else return 0;
}

char *classify(trainset_t *trainset, float_array_t values, int32_t k)
{
    if(values.size != trainset->records.data->values.size) return NULL;

    dv_t_array_t diff_values;
    array_init_dv_t(&diff_values);
    diff_values.comparator.compare = &dv_t_comparator;

    ARRAY_ITER(trainset->records, i) {
        record_t record = array_get_record_t(&trainset->records, i);
        float dw = 0.0f;
        for(int j = 0; j < values.size; ++j) {
            dw += array_get_float(&record.values, j) - array_get_float(&values, j);
        }
        dw *= dw;
        array_add_dv_t(&diff_values, (dv_t){dw, i});
    }
    
    array_sort_dv_t(&diff_values);

    hashmap_t hashmap;
    hashmap_init(&hashmap);

    for(int32_t i = 0; i < k; ++i) {
        dv_t dv = array_get_dv_t(&diff_values, i);
        record_t record = array_get_record_t(&trainset->records, dv.i);
        hashmap_add(&hashmap, record.classname.data, record.classname.size);
    }

    pair_t max_value = hashmap_max(&hashmap);
    char *result = (char *) malloc(sizeof(char) * max_value.length);
    strcpy(result, max_value.classname);

    array_free_dv_t(&diff_values);
    hashmap_free(&hashmap);
    return result;
}

float mapf(float number, float left_range, float right_range, float left_bound, float right_bound)
{
    return (number - left_range) / (right_range - left_range) * (right_bound - left_bound) + left_bound;
}

void gc_draw_pixel(void *display, point_t point, uint32_t color, point_t display_info)
{
    if((point.x >= 0 && point.y < display_info.x) && (point.y >= 0 && point.y < display_info.y)) {
        *((uint32_t *) display + (point.x + point.y * display_info.x)) = color;
    }
}

void gc_fill_rect(void *display, point_t point, point_t dimension, uint32_t color, point_t display_info)
{
    for(int32_t j = 0; j < dimension.y; ++j) {
        for(int32_t i = 0; i < dimension.x; ++i) {
            gc_draw_pixel(display, (point_t){point.x + i, point.y + j}, color, display_info);
        }
    }
}

void gc_draw_line(void *display, point_t start, point_t end, uint32_t color, point_t display_info)
{
    int dx = end.x - start.x;
    int dy = end.y - start.y;

    int steps = ABS(dx) > ABS(dy) ? ABS(dx) : ABS(dy);

    float Xinc = dx / (float) steps;
    float Yinc = dy / (float) steps;

    float fx = start.x;
    float fy = start.y;
    for(int i = 0; i <= steps; ++i) {
        gc_draw_pixel(display, (point_t){fx, fy}, color, display_info);
        fx += Xinc;
        fy += Yinc;
    }
}

DWORD WINAPI VisualThread(LPVOID lpParam)
{
    param_t *visual_params = (param_t *) lpParam;

    trainset_t *trainset = (trainset_t *) visual_params->x;
    trainset_t *test_trainset = (trainset_t *) visual_params->y;

    HINSTANCE hInstance = GetModuleHandleA(NULL);
    
    WNDCLASSW wClass = { 0 };
    wClass.hbrBackground = (HBRUSH)COLOR_WINDOW;
    wClass.hInstance = hInstance;
    wClass.hCursor = LoadCursor(NULL, IDC_ARROW);
    wClass.lpszClassName = L"Root";
    wClass.lpfnWndProc = WindowProc;
    
    if (!RegisterClassW(&wClass)) return -1;
    
    uint32_t Width = 800;
    uint32_t Height = 600;

    RECT window_rect = { 0 };
    window_rect.right = Width;
    window_rect.bottom = Height;
    window_rect.left = 0;
    window_rect.top = 0;

    AdjustWindowRect(&window_rect, WS_OVERLAPPEDWINDOW | WS_VISIBLE, 0);
    HWND window = CreateWindowW(
        wClass.lpszClassName,
        L"K-NN Classificator Visualization",
        WS_OVERLAPPEDWINDOW | WS_VISIBLE,
        CW_USEDEFAULT, CW_USEDEFAULT,
        window_rect.right - window_rect.left,
        window_rect.bottom - window_rect.top,
        NULL, NULL,
        NULL, NULL
    );

    GetWindowRect(window, &window_rect);

    uint32_t BitmapWidth = Width;
    uint32_t BitmapHeight = Height;

    uint32_t BytesPerPixel = 4;

    uint32_t TotalSize = BitmapWidth * BitmapHeight * BytesPerPixel;

    void *memory = VirtualAlloc(
        0,
        TotalSize,
        MEM_RESERVE | MEM_COMMIT,
        PAGE_READWRITE
    );

    
    BITMAPINFO BitmapInfo;
    BitmapInfo.bmiHeader.biSize = sizeof(BitmapInfo.bmiHeader);
    BitmapInfo.bmiHeader.biWidth = BitmapWidth;
    BitmapInfo.bmiHeader.biHeight = -BitmapHeight;
    BitmapInfo.bmiHeader.biPlanes = 1;
    BitmapInfo.bmiHeader.biBitCount = 32;
    BitmapInfo.bmiHeader.biCompression = BI_RGB;

    HDC hdc = GetDC(window);

    point_t display_info = (point_t){BitmapWidth, BitmapHeight};

    float last_accuracy = 0.0f;
    
    float k_width = BitmapWidth / (float)(trainset->records.size - 1);

    for(int32_t k = 1; k < trainset->records.size; ++k) {
        float accuracy = 0.0f;

        ARRAY_ITER(test_trainset->records, j) {
            record_t record = array_get_record_t(&test_trainset->records, j);
            char *classification = classify(trainset, record.values, k);
            if(!strcmp(classification, record.classname.data)) {
                accuracy += 1.0f;
            }
        }
        accuracy = accuracy / test_trainset->records.size;

        point_t start = (point_t){
            (int32_t)(k_width * (k - 1)),
            (int32_t)(BitmapHeight - last_accuracy * BitmapHeight)
        };
        point_t end = (point_t){
            (int32_t)(k_width * k),
            (int32_t)(BitmapHeight - accuracy * BitmapHeight)
        };
        gc_draw_line(memory, start, end, 0x00FF00FF, display_info);

        last_accuracy = accuracy;
        StretchDIBits(
            hdc, 0, 0,
            BitmapWidth, BitmapHeight,
            0, 0,
            BitmapWidth, BitmapHeight,
            memory, &BitmapInfo,
            DIB_RGB_COLORS,
            SRCCOPY
        );
    }

    MSG msg = { 0 };
    int32_t running = 1;
    while (running) {
        while (PeekMessageW(&msg, NULL, 0, 0, PM_REMOVE)) {
            switch (msg.message) {
                case WM_QUIT: {
                    running = 0;
                    break;
                }
            }
            TranslateMessage(&msg);
            DispatchMessageW(&msg);
        }

        StretchDIBits(
            hdc, 0, 0,
            BitmapWidth, BitmapHeight,
            0, 0,
            BitmapWidth, BitmapHeight,
            memory, &BitmapInfo,
            DIB_RGB_COLORS,
            SRCCOPY
        );
    }

    UnregisterClassW(wClass.lpszClassName, hInstance);
    VirtualFree(memory, 0, MEM_RELEASE);
    return 0;
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg) {
        case WM_DESTROY: {
            PostQuitMessage(0);
            break;
        }
        default: {
            return DefWindowProcW(hwnd, uMsg, wParam, lParam);
            break;
        }
    }
    return 0;
}