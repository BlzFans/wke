/*
     File:       TargetConditionals.h
 
     Contains:   Autoconfiguration of TARGET_ conditionals for Mac OS X
	 
                 Note:  TargetConditionals.h in 3.4 Universal Interfaces works
						with all compilers.  This header only recognizes compilers
						known to run on Mac OS X.
 
     Copyright:  (c) 2000-2004 by Apple Computer, Inc., all rights reserved.
 
*/

#ifndef __TARGETCONDITIONALS__
#define __TARGETCONDITIONALS__
/****************************************************************************************************

    TARGET_CPU_*    
    These conditionals specify which microprocessor instruction set is being
    generated.  At most one of these is true, the rest are false.

        TARGET_CPU_PPC          - Compiler is generating PowerPC instructions for 32-bit mode
        TARGET_CPU_PPC64		- Compiler is generating PowerPC instructions for 64-bit mode
        TARGET_CPU_68K          - Compiler is generating 680x0 instructions
        TARGET_CPU_X86          - Compiler is generating x86 instructions
        TARGET_CPU_MIPS         - Compiler is generating MIPS instructions
        TARGET_CPU_SPARC        - Compiler is generating Sparc instructions
        TARGET_CPU_ALPHA        - Compiler is generating Dec Alpha instructions


    TARGET_OS_* 
    These conditionals specify in which Operating System the generated code will
    run. At most one of the these is true, the rest are false.

        TARGET_OS_MAC           - Generate code will run under Mac OS
        TARGET_OS_WIN32         - Generate code will run under 32-bit Windows
        TARGET_OS_UNIX          - Generate code will run under some non Mac OS X unix 

        TARGET_OS_EMBEDDED      - Generate code will run under an embedded OS variant
                                  of one of the above OSes. Can be true at the same
                                  time as TARGET_OS_MAC

    TARGET_RT_* 
    These conditionals specify in which runtime the generated code will
    run. This is needed when the OS and CPU support more than one runtime
    (e.g. Mac OS X supports CFM and mach-o).

        TARGET_RT_LITTLE_ENDIAN - Generated code uses little endian format for integers
        TARGET_RT_BIG_ENDIAN    - Generated code uses big endian format for integers    
        TARGET_RT_64_BIT        - Generated code uses 64-bit pointers    
        TARGET_RT_MAC_CFM       - TARGET_OS_MAC is true and CFM68K or PowerPC CFM (TVectors) are used
        TARGET_RT_MAC_MACHO     - TARGET_OS_MAC is true and Mach-O/dlyd runtime is used

****************************************************************************************************/


    #define TARGET_OS_MAC               0
    #define TARGET_OS_WIN32             1
    #define TARGET_OS_UNIX              0
    #define TARGET_OS_EMBEDDED          0

    #define TARGET_CPU_68K              0
    #define TARGET_CPU_ALPHA            0
    #define TARGET_CPU_MIPS             0
    #define TARGET_CPU_PPC              0
    #define TARGET_CPU_PPC64            0
    #define TARGET_CPU_SPARC            0
    #define TARGET_CPU_X86              1
    #define TARGET_CPU_X86_64           0

    #define TARGET_RT_64_BIT            0
    #define TARGET_RT_BIG_ENDIAN        0
    #define TARGET_RT_LITTLE_ENDIAN     1
    #define TARGET_RT_MAC_CFM           0
    #define TARGET_RT_MAC_MACHO         0


#endif  /* __TARGETCONDITIONALS__ */
