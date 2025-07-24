<a href="https://www.unrealengine.com/">![Unreal Engine](https://img.shields.io/badge/Unreal-5.4%2B-dea309)</a>

<br/>
<p align="center">
  <a href="https://github.com/Bumvolla/MaskTools">
    <img src="Resources/Icon128.png" alt="Logo" width="80" height="80">
  </a>
<h3 align="center">Mask Tools</h3>

  <p align="center">
     Unreal editor extension with common tools for working with mask textures
    <br/>
    <br/>
    <a href="https://bumvolla.github.io/2025/04/20/MaskToolsDocs/"><strong>Docs</strong></a>
  </p>
  
## Introduction and motivations

Mask textures are usually used in game development and 3D art to compress up to 4 grayscale textures into a single RGBA texture. Using this method you can reduce drawcalls and improve your game performance. 

Texture masks are pretty usefull for 3D artists, VFX artists, Technical artists and kinda anyone working in a 3D environment. I've always been confused about why the creation method for this widespread workflow was so ofuscated, usually relying on paid 3rd party software or niche open source software so I thought could be cool to have tools to working with masks in-engine.

For more information refer to these documentation pages:

[Using texture masks](https://dev.epicgames.com/documentation/en-us/unreal-engine/using-texture-masks-in-unreal-engine)

## Installation

The plugin can be adquired in [Fab](https://www.fab.com/listings/0d7fb6db-b5ad-4375-9330-f659a633ffd1) or cloned or downloaded for free from [GitHub](https://github.com/Bumvolla/MaskTools).

If you got Git installed on your computer you can clone the repository by typing this command in a terminal in your engine "Plugins/Marketplace" folder or your project "Plugins" folder:


~~~
git clone https://github.com/Bumvolla/MaskTools.git
~~~

or, if you already got your unreal project in a git repository:


~~~
git submodule add https://github.com/Bumvolla/MaskTools.git
~~~



If not, you can download the .zip file in the [latest release](https://github.com/Bumvolla/MaskTools/releases/latest) and extract it in your project "Plugins" folder

## Features

### Texture Mixer

- Merge up to 4 grayscale textures into one single RGBA texture using an editor built-in dockeable tab.
- Choose one of up to 12 resize methods for your texture.
- Use the default content browser or the built in asset picker

<img width="1260" height="762" alt="MaskExample" src="https://github.com/user-attachments/assets/1f1ce3f0-4d5b-4619-b5c7-16c910fd5382" />

### Texture Splitter

Split any texture into it's RGBA channels, creating one single grayscale texture for each one of them. 

![gif](https://github.com/Bumvolla/bumvolla.github.io/blob/main/img/MaskToolsDocs/ChannelSplitter.gif)

## Contributing

Contributions to this project are welcome. If you find any issues or have suggestions for improvements, feel free to create an issue or submit a pull request.

