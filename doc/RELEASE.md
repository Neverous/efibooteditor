# Release process

1. Create tag for new version

    ```shell
    $ git tag -s vX.Y.Z -m ""
    $ git push tag vX.Y.Z
    ```

2. Cleanup `latest` release and tag from [GitHub](https://github.com/Neverous/efibooteditor/releases/tag/latest)

3. Create release for new tag through [GitHub UI](https://github.com/Neverous/efibooteditor/releases)