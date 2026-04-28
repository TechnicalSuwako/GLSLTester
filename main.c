#include <glad/glad.h>
#ifdef __APPLE__
#define GL_SILENCE_DEPRECATION
#include <CoreFoundation/CFBundle.h>
#endif
#include <GLFW/glfw3.h>
#include <stdio.h>
#include <stdlib.h>
#include <vector.h>
#include <math.h>

// ファイルパスを構築（プラットフォームに応じて）
char *constructFilePath(const char *basePath) {
  char *fullPath = (char *)malloc(512);
  if (!fullPath) {
    printf("エラー: ファイルパスのメモリ確保に失敗しました\n");
    return NULL;
  }

#ifdef __APPLE__
  // macOS: リソースフォルダからファイルパスを取得
  CFBundleRef mainBundle = CFBundleGetMainBundle();
  CFStringRef fileName = CFStringCreateWithCString(NULL, basePath, kCFStringEncodingUTF8);
  CFURLRef fileURL = CFBundleCopyResourceURL(mainBundle, fileName, NULL, NULL);
  CFRelease(fileName);
  if (!fileURL) {
    printf("エラー: リソースファイル %s が見つかりませんでした\n", basePath);
    free(fullPath);
    return NULL;
  }
  Boolean success = CFURLGetFileSystemRepresentation(fileURL, true, (UInt8 *)fullPath, 512);
  CFRelease(fileURL);
  if (!success) {
    printf("エラー: ファイルパス %s の変換に失敗しました\n", basePath);
    free(fullPath);
    return NULL;
  }
#else
  // WindowsとUnix: shaderサブディレクトリを仮定
  snprintf(fullPath, 512, "%s", basePath);
#endif

  printf("シェーダーファイルを読み込み中: %s\n", fullPath);
  return fullPath;
}

// ファイルからシェーダーソースを読み込む
char *readShaderFile(const char *filename) {
  char *fullPath = constructFilePath(filename);
  if (!fullPath) return NULL;

  FILE *file = fopen(filename, "r");
  if (!file) {
    printf("シェーダーファイル %s を開けませんでした\n", filename);
    free(fullPath);
    return NULL;
  }

  // ファイルサイズを取得
  fseek(file, 0, SEEK_END);
  long size = ftell(file);
  fseek(file, 0, SEEK_SET);

  // メモリを確保（終端文字を含む）
  char *source = (char *)malloc(size + 1);
  if (!source) {
    printf("シェーダーファイル %s のメモリ確保に失敗しました\n", filename);
    fclose(file);
    free(fullPath);
    return NULL;
  }

  // ファイルを読み込む
  size_t read_size = fread(source, 1, size, file);
  source[read_size] = '\0'; // 終端文字を追加
  fclose(file);
  free(fullPath);
  return source;
}

// シェーダーのコンパイルエラーをチェック
void checkShaderCompile(GLuint shader) {
  GLint success;
  glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
  if (!success) {
    char infoLog[512];
    glGetShaderInfoLog(shader, 512, NULL, infoLog);
    printf("シェーダーのコンパイルに失敗しました:\n%s\n", infoLog);
  }
}

// プログラムのリンクエラーをチェック
void checkProgramLink(GLuint program) {
  GLint success;
  glGetProgramiv(program, GL_LINK_STATUS, &success);
  if (!success) {
    char infoLog[512];
    glGetProgramInfoLog(program, 512, NULL, infoLog);
    printf("プログラムのリンクに失敗しました:\n%s\n", infoLog);
  }
}

// ビューポートのリサイズコールバック
void framebuffer_size_callback(GLFWwindow *window, int width, int height) {
  (void)window;
  glViewport(0, 0, width, height);
}

int main(int argc, char **argv) {
  (void)argc;
  (void)argv;

  // GLFWを初期化
  if (!glfwInit()) {
    puts("GLFWの初期化に失敗しました");
    return -1;
  }

  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
#ifdef __APPLE__
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
#else
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
#endif
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#ifdef __APPLE__
  glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

  // ウィンドウを作成
  GLFWwindow *window = glfwCreateWindow(800, 600, "OpenGLシェーダーテスト", NULL, NULL);
  if (!window) {
    puts("GLFWウィンドウの作成に失敗しました");
    glfwTerminate();
    return -1;
  }
  glfwMakeContextCurrent(window);

  // GLADを初期化
  if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
    puts("GLADの初期化に失敗しました");
    glfwDestroyWindow(window);
    glfwTerminate();
    return -1;
  }

  // ビューポートを設定
  glViewport(0, 0, 800, 600);
  glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

  // シェーダーファイルを読み込む
#ifdef __APPLE__
  char *vertexShaderSource = readShaderFile("shader/vertex.mac.glsl");
  char *fragmentShaderSource = readShaderFile("shader/fragment.mac.glsl");
#else
  char *vertexShaderSource = readShaderFile("shader/vertex.glsl");
  char *fragmentShaderSource = readShaderFile("shader/fragment.glsl");
#endif
  if (!vertexShaderSource || !fragmentShaderSource) {
    puts("シェーダーファイルの読み込みに失敗しました");
    if (vertexShaderSource) free(vertexShaderSource);
    if (fragmentShaderSource) free(fragmentShaderSource);
    glfwDestroyWindow(window);
    glfwTerminate();
    return -1;
  }

  // シェーダーをコンパイル
  GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
  glShaderSource(vertexShader, 1, (const char **)&vertexShaderSource, NULL);
  glCompileShader(vertexShader);
  checkShaderCompile(vertexShader);

  GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
  glShaderSource(fragmentShader, 1, (const char **)&fragmentShaderSource, NULL);
  glCompileShader(fragmentShader);
  checkShaderCompile(fragmentShader);

  // シェーダープログラムをリンク
  GLuint shaderProgram = glCreateProgram();
  glAttachShader(shaderProgram, vertexShader);
  glAttachShader(shaderProgram, fragmentShader);
  glLinkProgram(shaderProgram);
  checkProgramLink(shaderProgram);

  glDeleteShader(vertexShader);
  glDeleteShader(fragmentShader);
  free(vertexShaderSource);
  free(fragmentShaderSource);

  // 頂点データ（フルスクリーンクアッド）を設定
  float vertices[] = {
    -1.0f, -1.0f, 0.0f, 0.0f, // 左下（位置、UV）
     1.0f, -1.0f, 1.0f, 0.0f, // 右下
     1.0f,  1.0f, 1.0f, 1.0f, // 右上
    -1.0f,  1.0f, 0.0f, 1.0f  // 左上
  };
  GLuint indices[] = { 0, 1, 2, 2, 3, 0 };

  GLuint VAO, VBO, EBO;
  glGenVertexArrays(1, &VAO);
  glGenBuffers(1, &VBO);
  glGenBuffers(1, &EBO);

  glBindVertexArray(VAO);
  glBindBuffer(GL_ARRAY_BUFFER, VBO);
  glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

  // 頂点属性：位置（location 0）、UV（location 1）
  glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void *)0);
  glEnableVertexAttribArray(0);
  size_t offset = 2 * sizeof(float);
  glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void *)offset);
  glEnableVertexAttribArray(1);

  // メインレンダリングループ
  while (!glfwWindowShouldClose(window)) {
    // 画面をクリア
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    // シェーダープログラムを使用
    glUseProgram(shaderProgram);

    // 時間に基づいて色をアニメーション
    float time = (float)glfwGetTime();
    float speed = 1.0f; // アニメーションの速度
    float phase = 2.0f * 3.14159f / 3.0f; // // 120度の位相差（3色用）

    // uColorA: 赤 -> 緑 -> 青
    Vector3 colorA;
    colorA.r = sin(time * speed + 0.0f) * 0.5f + 0.5f; // 0.0 to 1.0
    colorA.g = sin(time * speed + phase) * 0.5f + 0.5f;
    colorA.b = sin(time * speed + 2.0f * phase) * 0.5f + 0.5f;

    // uColorB: 緑 -> 青 -> 赤
    Vector3 colorB;
    colorB.r = sin(time * speed + 2.0f * phase) * 0.5f + 0.5f;
    colorB.g = sin(time * speed + 0.0f) * 0.5f + 0.5f;
    colorB.b = sin(time * speed + phase) * 0.5f + 0.5f;

    // uColorC: 青 -> 赤 -> 緑
    Vector3 colorC;
    colorC.r = sin(time * speed + phase) * 0.5f + 0.5f;
    colorC.g = sin(time * speed + 2.0f * phase) * 0.5f + 0.5f;
    colorC.b = sin(time * speed + 0.0f) * 0.5f + 0.5f;

    // ユニフォームを設定
    glUniform3f(glGetUniformLocation(shaderProgram, "uColorA"), colorA.r, colorA.g, colorA.b); // 赤
    glUniform3f(glGetUniformLocation(shaderProgram, "uColorB"), colorB.r, colorB.g, colorB.b); // 緑
    glUniform3f(glGetUniformLocation(shaderProgram, "uColorC"), colorC.r, colorC.g, colorC.b); // 青

    // クアッドを描画
    glBindVertexArray(VAO);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

    // バッファをスワップし、イベントを処理
    glfwSwapBuffers(window);
    glfwPollEvents();
  }

  // クリーンアップ
  glDeleteVertexArrays(1, &VAO);
  glDeleteBuffers(1, &VBO);
  glDeleteBuffers(1, &EBO);
  glDeleteProgram(shaderProgram);
  glfwDestroyWindow(window);
  glfwTerminate();

  return 0;
}
