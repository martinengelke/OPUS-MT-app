# OPUS-MT

Fast and secure translation on your local machine with a GUI, powered by marian and Bergamot.
The app is an adaptation of [translateLocally](https://github.com/XapaJIaMnu/translateLocally)
and integrates publically available translation models from the [OPUS-MT project](https://github.com/Helsinki-NLP/OPUS-MT) to bring fast and secure machine translation to the desktop of end users.


## Downloads


You can download the latest automatic build for Windows, Linux and Mac from the [releases](https://github.com/Helsinki-NLP/OPUS-MT-app/releases) github tab.

The app is in development and we provide a preliminary test version with no guarantees.
The list of models will be extended in the near future. [Automatic builds](https://github.com/Helsinki-NLP/OPUS-MT-app/releases) are available from:

* [Mac OS Catalina 10.15](https://github.com/Helsinki-NLP/OPUS-MT-app/releases/download/latest/opusMT.macos-10.15.core-avx-i.dmg)
* [Mac OS Big Sur 11.0](https://github.com/Helsinki-NLP/OPUS-MT-app/releases/download/latest/opusMT.macos-11.0.core-avx-i.dmg)
* [Windows 2019](https://github.com/Helsinki-NLP/OPUS-MT-app/releases/download/latest/opusMT.windows-2019.core-avx-i.exe)
* [Windows 2022](https://github.com/Helsinki-NLP/OPUS-MT-app/releases/download/latest/opusMT.windows-2022.core-avx-i.exe)
* [Ubuntu 18.04](https://github.com/Helsinki-NLP/OPUS-MT-app/releases/download/latest/opusMT-v0.0.2+3bda6a2-Ubuntu-18.04.AVX.deb)
* [Ubuntu 20.04](https://github.com/Helsinki-NLP/OPUS-MT-app/releases/download/latest/opusMT-v0.0.2+3bda6a2-Ubuntu-20.04.AVX.deb)
* [Source zip](https://github.com/Helsinki-NLP/OPUS-MT-app/archive/refs/tags/latest.zip)
* [Source tar.gz](https://github.com/Helsinki-NLP/OPUS-MT-app/archive/refs/tags/latest.tar.gz)


## Compile from source

To build and run

```bash
mkdir build
cd build
cmake ..
make -j5
./opusMT
```

Requires `QT>=5 libarchive intel-mkl-static`. We make use of the `QT>=5 network`, `QT>=5 linguisticTool` and `QT>=5 svg` components. Depending on your distro, those may be split in separate package from your QT package (Eg `qt{6/7}-tools-dev`; `qt{5/6}-svg` or `libqt5svg5-dev`). QT6 is fully supported and its use is encouraged. `intel-mkl-static` may be part of `mkl` or `intel-mkl` packages.

## MacOS Build
On MacOS, opusMT doesn't rely on MKL, but instead on Apple accelerate. If you want to build a portable executable that is able to run on multiple machines, we recommend using Qt's distribution of Qt, as opposed to homebrew's due to issues with [macdeployqt](https://github.com/XapaJIaMnu/translateLocally/issues/69). To produce a `.dmg`do:
```bash
mkdir build
cd build
cmake ..
cmake --build . -j3 --target opusMT-bin opusMT.dmg
```
Alternatively, if you wish to sign and notarize the `.dmg`for distribution, you may use [macdmg.sh](dist/macdmg.sh)
```bash
mkdir build
cd build
cmake ..
make -j5
../dist/macdmg.sh .
```
Check the script for the environment variables that you need to set if you want to take advantage of signing and notarization.

## Windows Build
On Windows, we recommend using [vcpkg](https://github.com/Microsoft/vcpkg) to install all necessary packages and Visual Studio to perform the build.

# Command line interface

opusMT supports using the command line to perform translations. Example usage:

```bash
./opusMT --help
Usage: ./opusMT [options]
A secure translation service that performs translations for you locally, on your own machine.

Options:
  -h, --help                     Displays help on commandline options.
  --help-all                     Displays help including Qt specific options.
  -v, --version                  Displays version information.
  -l, --list-models              List locally installed models.
  -a, --available-models         Connect to the Internet and list available
                                 models. Only shows models that are NOT
                                 installed locally or have a new version
                                 available online.
  -d, --download-model <output>  Connect to the Internet and download a model.
  -r, --remove-model <output>    Remove a model from the local machine. Only
                                 works for models managed with opusMT.
  -m, --model <model>            Select model for translation.
  -i, --input <input>            Source translation text (or just used stdin).
  -o, --output <output>          Target translation output (or just used
                                 stdout).
```

## Downloading models from CLI
Models can be downloaded from the GUI or the CLI. For the CLI model management you need to:
```bash
$ ./opusMT -a

English-Finnish type: tiny version: 1; To download do -d eng-fin-tiny
Swedish-Finnish type: tiny version: 1; To download do -d swe-fin-tiny

$ ./opusMT -d eng-fin-tiny
Downloading English-Finnish type tiny...

100% [############################################################]
Model downloaded successfully! You can now invoke it with -m eng-fin-tiny
```

## Removing models from the CLI
Models can be removed from the GUI or the CLI. For the CLI model removal, you need to:
```bash
./opusMT -r eng-fin-tiny
Model English-Finnish type tiny successfully removed.
```

## Listing available models
The avialble models can be listed with `-l`
```bash
./opusMT -l
English-Finnish type: tiny version: 1; To invoke do -m eng-fin-tiny
Swedish-Finnish type: tiny version: 1; To invoke do -m swe-fin-tiny
```

## Translating a single sentence
Note that customising the translator settings can only be done via the GUI.
```bash
echo "Me gustaria comprar la casa verde" | ./opusMT -m es-en-tiny
```

## Translating a whole dataset
```bash
sacrebleu -t wmt13 -l en-es --echo ref > /tmp/es.in
./opusMT -m es-en-tiny -i /tmp/es.in -o /tmp/en.out
```

Note that if you are using the macOS opusMT.app version, the `-i` and `-o` options are not able to read most files. You can use pipes instead, e.g.
```bash
opusMT.app/Contents/MacOS/opusMT -m es-en-tiny < input.txt > output.txt
```

## Pivoting and piping
The command line interface can be used to chain several translation models to achieve pivot translation, for example Spanish to German.
```bash
sacrebleu -t wmt13 -l en-es --echo ref > /tmp/es.in
cat /tmp/es.in | ./opusMT -m es-en-tiny | ./opusMT -m en-de-tiny -o /tmp/de.out
```

### Using in an environment without a running display server
OPUS-MT is built on top of QT, with modules linked in such a way that a display server required in order for the program to start, even if we only use the command line interface, resulting in an error like:

```bash
$ ./opusMT --version
loaded library "/opt/conda/plugins/platforms/libqxcb.so"
qt.qpa.xcb: could not connect to display
qt.qpa.plugin: Could not load the Qt platform plugin "xcb" in "" even though it was found.
This application failed to start because no Qt platform plugin could be initialized. Reinstalling the application may fix this problem.

Available platform plugins are: eglfs, minimal, minimalegl, offscreen, vnc, webgl, xcb.

Aborted
```
To get around this, we can use set `QT_QPA_PLATFORM=offscreen` if we have the `offscreen` plugin:
```bash
QT_QPA_PLATFORM=offscreen ./opusMT --version
OPUS-MT v0.0.2+1cd4d20
```
OR use `xvfb` to emulate a server.
```bash
xvfb-run --auto-servernum ./opusMT --version
opusMT v0.0.2+a603422
```
Note that this issue only occurs on Linux, as Windows and Mac (at least to my knowledge) always have an active display even in remote sessions.

# Importing custom models
opusMT supports importing custom models. opusMT uses the [Bergamot](https://github.com/browsermt/marian-dev) fork of [marian](https://github.com/marian-nmt/marian-dev). As such, it supports the vast majority marian models out of the box. You can just train your marian model and place it a directory. 
## Basic model import
The directory structure of a opusMT model looks like this:
```bash
$ tree my-custom-model
my-custom-model/
├── config.intgemm8bitalpha.yml
├── model_info.json
├── model.npz
└── vocab.deen.spm
```
The `config.intgemm8bitalpha.yml` name is hardcoded, and so is `model_info.json`. Everything else could have an arbitrary name. opusMT will load the model according to the settings specified in `config.intgemm8bitalpha.yml`. These are just normal marian configuration options. `model_info.json` contains metadata about the model:
```bash
$ cat model_info.json 
{
  "modelName": "German-English tiny",
  "shortName": "de-en-tiny",
  "type":      "tiny",
  "src":       "German",
  "trg":       "English",
  "version":   2.0,
  "API":       1.0
}
```
Once the files are in place, tar the model:
```bash
$ tar -czvf my-custom-model.tar.gz my-custom-model
```
And you can import it via the GUI: Open opusMT and go to **Edit -> Translator Settings -> Languages -> Import model** and navigate to the archive you created. 

## Quantising the model
The process described above will create a model usable by opusMT, albeit not a very efficient one. In order to create an efficient model we recommend that you quantise the model to 8-bit integers. You can do that by downloading and compiling the [Bergamot](https://github.com/browsermt/marian-dev) fork of marian, and using `marian-conv` to create the quantised model:
```bash
$MARIAN/marian-conv -f input_model.npz -t output_model.bin --gemm-type intgemm8
```
And then changing your configuration `config.intgemm8bitalpha.yml` to point to this new model, as well as appending `gemm-precision: int8shift` to it.

## Further increasing performance
**For best results, we strongly recommend that you use student models.** Instructions on how to create one + scripts can be found [here](https://github.com/browsermt/students/tree/master/train-student) and a detailed video tutorial and explanations are available [here](https://nbogoychev.com/efficient-machine-translation/). Student models are typically at least 8X faster than teacher models such as the transformer-base preset.

You can further achive another 30\%-40\% performance boost if you precompute the quantisation multipliers of the model and you use a lexical shortlist. The process for those is described in details at the Bergamot project's [Github](https://github.com/browsermt/students/tree/master/train-student#5-8-bit-quantization). Remember that you need to use the [Bergamot](https://github.com/browsermt/marian-dev) fork of Marian.

Example script that converts a marian model to the most efficient 8-bit representation can also be found at Bergamot's [Github](https://github.com/browsermt/students/blob/master/esen/esen.student.tiny11/speed.cpu.intgemm8bitalpha.sh).


# Acknowledgements

<img src="https://raw.githubusercontent.com/XapaJIaMnu/translateLocally/master/eu-logo.png" data-canonical-src="https://raw.githubusercontent.com/XapaJIaMnu/translateLocally/master/eu-logo.png" width=10% />

## Bergamot

The bergamot project has received funding from the European Union’s Horizon 2020 research and innovation programme under grant agreement No 825303.

This project was made possible through the combined effort of all researchers and partners in the Bergamot project https://browser.mt/partners/ . The translation models are prepared as part of the Bergamot project https://github.com/browsermt/students . The translation engine used is https://github.com/browsermt/bergamot-translator which is based on marian https://github.com/marian-nmt/marian-dev .


## OPUS-MT

The OPUS-MT project received funding  by the [European Language Grid](https://www.european-language-grid.eu/) as [pilot project 2866](https://live.european-language-grid.eu/catalogue/#/resource/projects/2866), is supported by the [FoTran project](https://www.helsinki.fi/en/researchgroups/natural-language-understanding-with-cross-lingual-grounding), funded by the European Research Council (ERC) under the European Union’s Horizon 2020 research and innovation programme (grant agreement No 771113), and the [MeMAD project](https://memad.eu/), funded by the European Union’s Horizon 2020 Research and Innovation Programme under grant agreement No 780069. We are also grateful for the generous computational resources and IT infrastructure provided by [CSC -- IT Center for Science](https://www.csc.fi/), Finland.
