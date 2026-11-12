# CI/CD

## ci ワークフロー

### 対象プラットフォーム

以下のプラットフォームでのプロセスをそれぞれ JOB として実装します。

- ubuntu-latest
- macos-26
- linux aarch64 on macos-26 + qemu
- windows-latest + MSYS2
- freebsd13 on ubuntu-latest + qemu

### ビルドツール

#### 共通依存関係

- **Git**: ソースコード管理
- **Make/GMake**: ビルドシステム
- **C/C++コンパイラ**: gcc/clang/mingw-gcc

#### プラットフォーム別依存関係

##### Ubuntu/Linux

- **cmake**: mozjpeg ビルド用
- **nasm**: アセンブリ最適化
- **valgrind**: メモリリーク検出（オプション）

##### macOS

- **cmake**: mozjpeg ビルド用（brew install cmake）
- **nasm**: アセンブリ最適化（brew install nasm）
- **leaks**: メモリリーク検出（標準搭載）

##### FreeBSD

- **cmake**: mozjpeg ビルド用（pkg install cmake）
- **nasm**: アセンブリ最適化（pkg install nasm）
- **gmake**: GNU Make（pkg install gmake）
- **valgrind**: メモリリーク検出（pkg install valgrind、オプション）

##### Windows (MSYS2/MinGW)

- **mingw-w64-x86_64-gcc**: GCC コンパイラ
- **mingw-w64-x86_64-cmake**: CMake ビルドシステム
- **mingw-w64-x86_64-nasm**: アセンブリ最適化
- **drmemory**: メモリリーク検出（オプション）

#### mozjpeg

- **バージョン**: v4.1.5（固定）
- **ビルドタイプ**: Release（最適化有効、デバッグシンボルなし）
- **設定**:
  - WITH_JPEG8=1（JPEG8 互換）
  - ENABLE_SHARED=0（共有ライブラリ無効）
  - ENABLE_STATIC=1（静的ライブラリ有効）

#### iqa

- **ビルドタイプ**: Release
- **依存**: なし（自己完結型）

#### jpeg-archive

- **最適化レベル**: -O3
- **C 標準**: C99
- **依存**: mozjpeg, iqa

### プロセス

0. 環境構築 (キャッシュ活用)
1. リポジトリのクローン
2. mozjpeg のビルド make build (v4.1.5 固定版・キャッシュ活用)
3. iqa のビルド (release)
4. jpeg-archive のビルド (release)
5. libjpegarchive.a のビルド make libjpegarchive.a
6. iqa のテスト cd src/iqa && make test && cd build/release/test && ./test
7. iqa のメモリリークテスト cd src/iqa && ./memory-test.sh
8. ユニットテストの実行 make test
9. libjpegarchive テストの実行 ./test/libjpegarchive
10. 統合テストの実行 cd test && ./test.sh
11. メモリリークテストの実行 ./test/memory_leak_test.sh（全プラットフォーム対応）
12. クリーンビルドテスト make clean && make all && make test
13. バイナリサイズとファイルサイズ削減率の確認
