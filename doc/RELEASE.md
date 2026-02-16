# Release process

1. Draft release for new tag through [GitHub UI](https://github.com/Neverous/efibooteditor/releases)

2. Create tag for new version

    ```shell
    git tag -s vX.Y.Z -m ""
    git push tag vX.Y.Z
    ```

3. Monitor the asset preparation in GHA, it should publish the draft after it's done

## Update [AUR](https://aur.archlinux.org/packages/efibooteditor)

1. Bump version in PKGBUILD

2. Update b2sums: `makepkg -g`

3. Update SRCINFO: `makepkg --printsrcinfo > .SRCINFO`

4. Commit and push changes to AUR

    ```shell
    git commit -m "Update to vX.Y.Z"
    git push
    ```
