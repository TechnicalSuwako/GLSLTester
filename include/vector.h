#pragma once

typedef union {
  struct {
    float x;
    float y;
  };
  struct {
    float u;
    float v;
  };
} Vector2;

typedef union {
  struct {
    float x;
    float y;
    float z;
  };
  struct {
    float r;
    float g;
    float b;
  };
} Vector3;

typedef union {
  struct {
    float x;
    float y;
    float z;
    float w;
  };
  struct {
    float r;
    float g;
    float b;
    float a;
  };
} Vector4;
