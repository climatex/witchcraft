# witchcraft
Having fun while experimenting - do not use these in production code.

*goto/goto.h*: A global goto example (x86 only), similar to std::longjmp.
This one deliberately ignores execution context and stack frames - hilarity ensues.
Check out [goto.cpp](https://github.com/climatex/witchcraft/blob/master/goto/goto.cpp) for an example.

*private*: Read/write of private member variables outside of a class, using memory access.
Works with classes without a vftable (virtual functions) and proper boundary alignment.

*nixie*: Off-line, AC-synchronized nixie clock with an Arduino Mega2560 and optional DCF module.
Check out [my website](http://boginjr.com/electronics/lv/nixie-clock/) for more details.
