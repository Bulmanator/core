#define CORE_MODULE
#define PNG_MODULE

#include "core.h"
#include "png.h"

#include <stdio.h>

int main(int argc, char **argv) {
    if (argc < 2) {
        printf("usage: %s <image>.png\n", argv[0]);
        return 1;
    }

    Log_Init();

    M_Temp temp = M_AcquireTemp(0, 0);

    Str8 path = Sz(argv[1]);
    PNG_DecodeFlags flags = PNG_DECODE_FLAG_VALIDATE_CRC;

    PNG_Image image = { 0 };
    if (!PNG_DecodeFromPath(temp.arena, &image, path, flags)) {
        printf("Failed to decode PNG image\n");

        Log_MessageArray messages = Log_PopScope(temp.arena);
        for (U32 it = 0; it < messages.count; ++it) {
            Log_Message *msg = &messages.items[it];

            if (msg->code == LOG_ERROR) {
                printf("%d:%.*s: %.*s\n", msg->line, Sv(msg->func), Sv(msg->message));
            }
        }
    }
    else {
        Str8 basename;
        basename = Str8_GetBasename(path);
        basename = Str8_StripExtension(basename);

        Str8 raw_path = Sf(temp.arena, "%.*s.raw", Sv(basename));

        printf("Successfully decoded PNG image!\n");
        printf("    ... image dimensions are: %dx%d\n", image.width, image.height);
        printf("    ... dumping raw pixels to %.*s\n", Sv(raw_path));

        OS_Handle file = FS_OpenFile(raw_path, FS_ACCESS_WRITE);

        S64 pixel_size = (image.bit_depth >> 3) * image.channels;

        Str8 data;
        data.count = pixel_size * image.width * image.height;
        data.data  = image.pixels;

        S64 written = FS_WriteFile(file, data, 0);
        if (written != data.count) {
            printf("There was an issue writing the file\n");

            Log_MessageArray messages = Log_PopScope(temp.arena);
            for (U32 it = 0; it < messages.count; ++it) {
                Log_Message *msg = &messages.items[it];

                if (msg->code == LOG_ERROR) {
                    printf("%d:%.*s: %.*s\n", msg->line, Sv(msg->func), Sv(msg->message));
                }
            }
        }
    }

    M_ReleaseTemp(temp);
    return 0;
}
