#include <cstdio>
#include <cstring>
#include <cmath>
#include "scene.hpp"
#include "defer.hpp"


void FillOutDefaultValues(scene* s) {
    *s = { 0 };

    s->CameraFOV = 90;
    s->CameraPos = {-2, -2, -2};
    s->ResX = 32;
    s->ResY = 32;
    s->SamplesPerPixel = 1;
}

inline bool IsWhitespace(char c) {
    return (c == ' '  || c == '\n' || c == '\r' || c == '\t');
}

void EatUntilWord(char* Str, uint32_t* ParsePos) {
    while (IsWhitespace(Str[*ParsePos]))
        ++(*ParsePos);
}

char* ReadEntireFile(const char* filename) {
    char* ret = nullptr;
    size_t str_len;
    FILE *fp = fopen(filename, "rb");
    if (fp) {
        defer {
            fclose(fp);
        };

        /* Go to the end of the file. */
        if (fseek(fp, 0L, SEEK_END) == 0) {
            /* Get the size of the file. */
            str_len = ftell(fp) + 1;
            if (str_len == 0) {
                fprintf(stderr, "Empty file");
                ret = (char*)malloc(1);
                ret[0] = '\0';
                return ret;
            }

            /* Go back to the start of the file. */
            if (fseek(fp, 0L, SEEK_SET) != 0) {
                fprintf(stderr, "Error reading file");
                return nullptr;
            }

            /* Allocate our buffer to that size. */
            ret = (char*)calloc(str_len, sizeof(char));

            /* Read the entire file into memory. */
            str_len = fread(ret, sizeof(char), str_len, fp);

            ret[str_len] = '\0';
            if (ferror(fp) != 0) {
                fprintf(stderr, "Error reading file");
                return nullptr;
            }
        }
    } else {
        fprintf(stderr, "Cannot read file: %s", filename);
        return nullptr;
    }

    return ret;
    /* Don't forget to call free() later! */
}

float3 ParseFloat3(char* Str, uint32_t* ParsePos) {
    float3 Result = { 0 };
    int CharsRead = 0;
    sscanf(Str+(*ParsePos), "%f, %f, %f%n",
           &Result.x, &Result.y, &Result.z, &CharsRead);

    *ParsePos += CharsRead;
    return Result;
}

float ParseFloat(char* Str, uint32_t* ParsePos) {
    float Result = 0;
    int CharsRead = 0;
    sscanf(Str+(*ParsePos), "%f%n", &Result, &CharsRead);
    *ParsePos += CharsRead;
    return Result;
}

int ParseInt(char* Str, uint32_t* ParsePos) {
    int Result = 0;
    int CharsRead = 0;
    sscanf(Str+(*ParsePos), "%d%n", &Result, &CharsRead);
    *ParsePos += CharsRead;
    return Result;
}

char* ParseFileName(char* Str, uint32_t* ParsePos) {
    uint32_t EndOfNameIdx = *ParsePos;
    while (!IsWhitespace(Str[EndOfNameIdx]))
        ++EndOfNameIdx;


    uint32_t Length = EndOfNameIdx - *ParsePos;
    char* Name = Str + *ParsePos;
    *ParsePos = EndOfNameIdx;

    char* Result = (char*)malloc(Length+1);
    memcpy(Result, Name, Length);
    Result[Length] = '\0';

    return Result;
}

void ParseSetting(char* Str, uint32_t* ParsePos, scene* Scene) {
    uint32_t EndOfNameIdx = *ParsePos;
    while ((Str[EndOfNameIdx] >= 'a' && Str[EndOfNameIdx] <= 'z') ||
           (Str[EndOfNameIdx] >= 'A' && Str[EndOfNameIdx] <= 'Z'))
        ++EndOfNameIdx;

    uint32_t Length = EndOfNameIdx - *ParsePos;
    char* Name = Str + *ParsePos;
    *ParsePos = EndOfNameIdx;

    EatUntilWord(Str, ParsePos);
    if (Str[*ParsePos] != '=') {
        fprintf(stderr, "ERROR: Syntax error, expected = after setting name (%.*s) Instead: '%s'\n", Length, Name, Str+(*ParsePos));
        return;
    }
    ++(*ParsePos);
    EatUntilWord(Str, ParsePos);

    if (Str[*ParsePos] == '\0') {
        fprintf(stderr, "ERROR: EOF while parsing setting %s\n", Name);
        return;
    }

#define IsSetting(SettingName) ((sizeof(SettingName)-1 == Length) &&    \
                                (strncmp(SettingName, Name, Length) == 0))

    if (IsSetting("GridFile")) {
        Scene->VolInfo.Grid = ReadXYZFile(ParseFileName(Str, ParsePos));
    } else if (IsSetting("Extinction")) {
        Scene->VolInfo.Extinction = ParseFloat3(Str, ParsePos);
    } else if (IsSetting("ScatteringAlbedo")) {
        Scene->VolInfo.ScatteringAlbedo = ParseFloat3(Str, ParsePos);
    } else if (IsSetting("G")) {
        Scene->VolInfo.G = ParseFloat3(Str, ParsePos);
    } else if (IsSetting("CameraPos")) {
        Scene->CameraPos = ParseFloat3(Str, ParsePos);
    } else if (IsSetting("CameraLookAt")) {
        Scene->CameraLookAt = Normalize(ParseFloat3(Str, ParsePos));
    } else if (IsSetting("SamplesPerPixel")) {
        Scene->SamplesPerPixel = ParseInt(Str, ParsePos);
    } else if (IsSetting("Seed")) {
        Scene->Seed = ParseInt(Str, ParsePos);
    } else if (IsSetting("CameraFOV")) {
        Scene->CameraFOV = ParseFloat(Str, ParsePos) / 180 * M_PI;
    } else if (IsSetting("ResX")) {
        Scene->ResX = ParseInt(Str, ParsePos);
    } else if (IsSetting("ResY")) {
        Scene->ResY = ParseInt(Str, ParsePos);
    } else {
        fprintf(stderr, "ERROR: Unknown Setting [%.*s] (length %u)", Length, Name, Length);
    }

#undef IsSetting
}

scene LoadSceneFromFile(const char* FilePath) {
    scene Result;
    FillOutDefaultValues(&Result);
    Result.SceneFile = FilePath;

    char* SceneConfStr = ReadEntireFile(FilePath);
    if (!SceneConfStr) {
        fprintf(stderr, "ERROR: scene file %s could not be read\n", FilePath);
        return {};
    }

    uint32_t ParsePos = 0;

    EatUntilWord(SceneConfStr, &ParsePos);
    while (SceneConfStr[ParsePos] != '\0') {
        uint32_t OldParsePos = ParsePos;
        ParseSetting(SceneConfStr, &ParsePos, &Result);
        EatUntilWord(SceneConfStr, &ParsePos);

        if (OldParsePos == ParsePos) {
            fprintf(stderr, "ERROR: No parsing progress could be made\n");
            return {};
        }
    }

    printf("INFO: SceneFile: %s\n", Result.SceneFile);

    printf("INFO: Extinction: %f %f %f\n",
           Result.VolInfo.Extinction.x,
           Result.VolInfo.Extinction.y,
           Result.VolInfo.Extinction.z);
    printf("INFO: ScatteringAlbedo: %f %f %f\n",
           Result.VolInfo.ScatteringAlbedo.x,
           Result.VolInfo.ScatteringAlbedo.y,
           Result.VolInfo.ScatteringAlbedo.z);
    printf("INFO: G: %f %f %f\n",
           Result.VolInfo.G.x,
           Result.VolInfo.G.y,
           Result.VolInfo.G.z);

    printf("INFO: CameraPos: %f %f %f\n",
           Result.CameraPos.x,
           Result.CameraPos.y,
           Result.CameraPos.z);
    printf("INFO: CameraLookAt: %f %f %f\n",
           Result.CameraLookAt.x,
           Result.CameraLookAt.y,
           Result.CameraLookAt.z);
    printf("INFO: CameraFOV: %f\n", Result.CameraFOV);
    printf("INFO: ResX: %d\n", Result.ResX);
    printf("INFO: ResY: %d\n", Result.ResY);

    return Result;
}
