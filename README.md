# Calculator app for Epsilon

This is a basic four-function calculator app for a NumWorks calculator, built with the Epsilon SDK.

Use the number keys and `+`, `−`, `×`, or `÷` to enter an operation. Press `EXE` or `OK` to calculate, `DEL` to erase the last character, and `Back` to clear.

## Build the app

To build this sample app, you will need to install the [embedded ARM toolchain](https://developer.arm.com/Tools%20and%20Software/GNU%20Toolchain) and [Node.js](https://nodejs.org/en/). The C SDK for Epsilon apps is shipped as an npm module called [nwlink](https://www.npmjs.com/package/nwlink) that will automatically be installed at compile time.

```shell
brew install numworks/tap/arm-none-eabi-gcc node # Or equivalent on your OS
make
```

You should now have a `target/calculator.nwa` file that you can distribute! Anyone can install it on their calculator from the [NumWorks online uploader](https://my.numworks.com/apps).

## Run the app locally

To run the app on your development machine, you can use the following command

```shell
# Now connect your NumWorks calculator to your computer using the USB cable
make run
```

## License

This sample app is distributed under the terms of the BSD License. See LICENSE for details.

## Trademarks

NumWorks is a registered trademark.
