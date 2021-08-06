/*
    Stubs that are required for gcc to link, but never actually called. 
    Some of these are ABI defined, hence why they're in the arch directory
*/

extern "C"
{
    void __cxa_atexit()
    {} //I mean if the kernel is 'exiting', i'm not too worried about calling dtors.

    void __cxa_throw_bad_array_new_length()
    {} //gcc being sneaky here, even with exceptions disabled at all stages.
}
