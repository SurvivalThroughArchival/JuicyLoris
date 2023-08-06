# JuicyLoris
Spectral Morphing Library from early 00's

See: http://www.cerlsoundgroup.org/Loris/

This repo is an attempt to Archive and modernise an old Spectral Morphing library called Loris, historically it existed as an application called Lemur. (Which is hard to find any trace of on the internet)

This library is also used in the Kyma hardware sound computer; where there's a real time implementation that uses .SPC files. The Loris library creates .SDIF files, that once analysis has been done, it's quicker to synthesise from. However it's not the ideal format for block based audio processing, as the timing envelope brakpoints are not evenly spaced. There is a SCP File.cpp in the source, which enables writing to SPC. This is probably the best best to explore how to do real time morphing from; a la Kyma.   

# BUILD INSTRUCTIONS:
```
cd into folder

cmake -B builds

cmake --build Builds
```
And you will have an exectutable in the Builds folder called "HelloLorris-0.0.0"

# RATIONALE:

A number of people have tried to make the Spectral Morphing library work on Windows and Mac, and failed at the arcane gibberish that is AutoMake/AutoConf. The original Package did a lot, including making a Python library, and CSound opcodes. This is not the intention here. 

I wanted a conscise easy to use static library that could be included in Juce Projects, or other C++ projects, and be made with CMake and more modern C++ standards. 

The moving over to modern C++ standards will be gradual, and I'd appreciate any help or advice. I have started, the first mission was to make the library with CMake instead of Auto Conf/AutoMake. 
The other parts were making it at least compile and work by replacing depracted/removed parts of the C++ language, such as changing the auto_ptr to using unique_ptr, and a few other edits. 

There's a reliance on iterators, as the c++ version used to originally write the library didn't have range based for loops.

There's a number of iffy bits in the code, seemingly lots of raw pointers.

# Reccomended settings for debugging/testing/development

Uncomment these lines in the CMakeLists.txt file to see the hideuousness:

```
target_compile_options(AudioPluginExample PRIVATE -Wall)
target_compile_options(AudioPluginExample PRIVATE -Werror -Wextra)
```

This will hammer out the faults in the library, there's many type issues. And of course it's using 32bit. So 64bit might be worth addressing/looking into. 

Changing to release mode seems to completely break things, runs through the code in 1.5 seconds (compared to 9/10 seconds) and leaves you with a mangled piece of audio. Debug mode seems to save the day somehow, and I don't exactly know why yet. 

# To JUCE devs that don't like or use CMAKE

Look it's all just source files anyway, just dump it in the producer if you don't want to faff with CMAKE

# Recent changes:

So many changes don't know where to start. But it compiles without a hitch with optimisations on, and before the compiler was at a loss, and garbling the output, albeit very quickly. 

Optimisations now work, and the time is down from 10 seconds to 0.6 seconds on M1 for the exectuable. In Juce running on a seperate thread it takes 1.6 seconds for a morph, of 2samples around 3 seconds long. (I don't know why there's a discrepency in time there)

The time is how long it takes for two samples of 3seconds long, to be analysed, time stretched, dialted and resynthesis of the partials. 

Some imrpovement on speed could be made. I had it running at 0.4seconds at one stage, before static casting lots of wrong type comparisons. The compiler was able to catch them, and think saved sometime compared to casting. So maybe there's a risk that could be taken there, un-doing all the static cast, or figuring if that is really causing a slight slow down. 

Tested on M1 and MacOS 10.13, so if you have Cmake installed you should be good to go. Much faster on M1 unsurprisingly. 

This library can work with FFTW but I'm going to have to fugure out building that statically and linking to Loris, as apparently that could be a speed improvement. 

Not all errors are accounted for, I've done my best, but there's still unused functions (that are actually used), some assigning result to itself stuff, some unused parameters. 

# Next Steps:

Carrying on optimisations on library, but in a reasonable state for now. 

Explore adding FFTW

Upload basic Juce example, or the whole plugin I've been working on

Figure out displaying the partials in the envelopes, and how to get inside the iterators for the paritals to display some relevant information/track partials over time. 

Figure out resynthesis in real time. So either just block load parts of the SDIF file, and synth from that. Or read partials and map directly to additive synth. Or look at the spc file,
as that's what the real time paper on it suggests doing, the SDIF isn't block based, or not equally spaced, and the spc file is a better match for block based processing I think, I haven't had a look yet. 

So get more relevant information about the partials and envelopes, and try and side step that abstraction with break point envelopes into a normalised parameter that can be updated or respond to real time interaction. But that requires unpacking the what this managerie of fuck whittery that is the morph class abstraction. 

# Quick Notes to potentially have a ganders at:

Deprecated in C++11 unary_function, binary_function, not1. Sprinkled about in Partial, Maker and Breakpoint files. 

Progress on unary function, and not1, not1 can be replaced by not_fn. Unary can be replaced with std::function, with a little re-write std::function<bool(object)>

I don't know what to do with binary function, the template is described as in1, in2, and return value (first arg, second arg, and result technically : https://en.cppreference.com/w/cpp/utility/functional/binary_function)

People have suggested on StackOverflow the replacement is a lambda, and unary function and binary function can be replaced with lambdas. Though i'm unsure (currently) how exactly to go about that.

useful links: https://stackoverflow.com/questions/70523732/unary-function-and-binary-function-are-deprecated-in-c-11-and-removed-in-c

and: https://stackoverflow.com/questions/33114656/replacement-for-stdbinary-function

One of the last comments down the page suggests just removing the binary function call and just use Lambdas. Which seems plausible to port to. 

std Function will likely need replacing as it too will be depreated. For now this is a quick fix to quiet the warnings; which is the current mission. 

These are lines I've made a quick note of, just noting things down, not set in stone they actually wrong, just worth having a look it for any signs or bad code smells:

//AiffData.cpp - Ln: 65 + 66, fn ln:87 (91,92), 126, // 127, 129, 130,// 154,155,156

//aiffFile.cpp - 129, 140 141,, 191 195

//analyser.cpp - 209, 521, 522, 562, 605, 666 667

//BigEndian.cpp - fn;72, fn:90, fn:120

//Collator.cpp - 151, 152

//Distiller.cpp - 331, 337, 340, 375

//F0 estimate.cpp - 341, 347, 356, 626 657, 673, 686, 689

//fftsg.cpp - FN: 299, 319, 363 (+ 32 other functions)

//fourierTransform.cpp - 413, 427

//frequency reference.cpp -  212

//Fundamental.cpp - 296,297, 344, 345, 451

//import lemur not checked

//linearEnv - 74, 94, 110, 127

//loris.h - 129, 300, 306,312,  316

//marker.cpp - 153 has std::unary function, is that deprecated?

//morpher.cpp - 170, 507, 522

//notifier not checked

//oscillator.cpp - 155

//partial.cpp - 147

//partialBuilder.cpp - 271
