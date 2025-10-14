# Tests

Tests are written with Google Test framework. Apart from standard arguments, the test executable supports the following custom arguments:

```
Scene Preview:
  --preview_on_failure
      Show a preview window if a scene test fails. Can also be set with
      PREVIEW_ON_FAILURE environment variable.
  --preview_always
      Always show a preview window for each scene test. Can also be set
      with PREVIEW_ALWAYS environment variable.
```

By default, the preview window is not shown.

<img width="502" height="502" alt="image" src="https://github.com/user-attachments/assets/6e59aee7-92f5-46ec-b7d4-66f477ae7512" />

The preview window is opened at the end of the test from fixture's `TearDown()` method. The test is paused until the window is closed.
