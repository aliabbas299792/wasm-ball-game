# Test Ball Game
A small test ball game that I'll be playing around with.
Uses the model branch from the [WASM Template](https://github.com/aliabbas299792/openglWASMTemplate/tree/model) template I made.

To compile it for development, simply run `./compile.sh`, and for release just run `./compile.sh release` in the public directory.

To serve the files first run `node install` or `yarn` in the main directory, and then just run `node index.js` and go to `localhost:2000` to see it running.

The blue tiles are made using instancing and the ball is rotated via quaternions.

A live example (might take a while to load): [Test](https://erewhon.xyz/3dexperiments/test/)
