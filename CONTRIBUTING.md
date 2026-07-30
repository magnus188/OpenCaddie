# Contributing to OpenCaddie

Thank you for helping build an open golf computer.

1. Open an issue for behavior or architecture changes.
2. Create a focused branch from `main`.
3. Use C++20 and the existing module boundaries; hardware access belongs behind
   an interface with a mock implementation.
4. Run `cmake --build --preset desktop --parallel`,
   `ctest --preset desktop`, `opencaddie_qmllint`, and
   `update_translations`.
5. Add tests for scoring, persistence, package validation, or transforms when
   changing those areas.
6. Submit a pull request with screenshots for 800×480 UI changes.

Never add Wi-Fi passwords, precise private routes, API tokens, or personal
scorecards to fixtures or logs. Contributions are licensed under
AGPL-3.0-or-later.

