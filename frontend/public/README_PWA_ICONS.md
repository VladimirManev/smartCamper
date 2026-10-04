# PWA / static icons (`public/`)

`public/` holds only files that must be available at a fixed root URL
(PWA manifest, favicon, Apple touch icon). App UI images live in `src/assets/`
and are imported from components.

For proper PWA support, add these PNG icons here:

- `pwa-192x192.png` - 192x192
- `pwa-512x512.png` - 512x512
- `apple-touch-icon.png` - 180x180 (iOS)

Tools:

- https://realfavicongenerator.net/
- https://www.pwabuilder.com/imageGenerator

Or convert `icon.svg` with ImageMagick / an online converter.
