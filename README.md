[![Build Status](https://travis-ci.org/reinterpretcat/utymap.svg?branch=master)](https://travis-ci.org/reinterpretcat/utymap)
[![Join the chat at https://gitter.im/reinterpretcat/utymap](https://badges.gitter.im/reinterpretcat/utymap.svg)](https://gitter.im/reinterpretcat/utymap?utm_source=badge&utm_medium=badge&utm_campaign=pr-badge&utm_content=badge)
<h2> Description </h2>

UtyMap is a library which provides highly customizable API for procedural world generation based on real map data, e.g. OpenStreetMap, NaturalEarth. Core logic is written on C++11 and can be used on many platforms as it has no dependency to specific game engine or application framework. 

<h2> Project structure </h2>
Project consists of two sub-projects:
<ul>
    <li><b>core</b> contains essential logic written on C++11 to build library for constructing of map based apps: terrain/buildings/osm-objects mesh generators, mapcss parser, spatial geo index, etc. </li>
    <li><b>unity</b> contains examples written on C# which can be reused to build map oriented Unity apps using core library. It will demonstrate basic use cases: globe zoom level rendering, 3D scene with all details.</li>
</ul>

<h2> Project status </h2>
<p> Project is under development, more details will be given later. </p>

<h2> Documentation </h2>
[Wiki](https://github.com/reinterpretcat/utymap/wiki)
