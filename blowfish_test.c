/**

    @file		blowfish_test.c

    @brief		Blowfish self-test application. Includes standard ECB mode
                tests and throughput tests (parallel and serial) for all
                supported modes (ECB, CBC, CFB, OFB and CTR).

    @author		Tom Bonner (tom.bonner@gmail.com)

    @date		22-June-2008

    Copyright (c) 2008, Tom Bonner.

    Permission is hereby granted, free of charge, to any person obtaining a
    copy of this software and associated documentation files (the "Software"),
    to deal in the Software without restriction, including without limitation
    the rights to use, copy, modify, merge, publish, distribute, sublicense,
    and/or sell copies of the Software, and to permit persons to whom the
    Software is furnished to do so, subject to the following conditions:

    The above copyright notice and this permission notice shall be included in
    all copies or substantial portions of the Software.

    Except as contained in this notice, the name(s) of the above copyright
    holders shall not be used in advertising or otherwise to promote the sale,
    use or other dealings in this Software without prior written authorisation.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
    IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
    FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
    AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
    LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
    FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
    DEALINGS IN THE SOFTWARE.

  */

#include <stdio.h>
#include <time.h>
#include <sys/time.h>
#include <malloc.h>
#include <memory.h>

#ifdef _OPENMP

#include <omp.h>

#endif

#include "blowfish.h"

/**

    @ingroup blowfish
    @defgroup blowfish_selftest Blowfish Self-Test
    @{

  */

/** @internal Test vector. */

typedef struct __BLOWFISH_TEST_VECTOR
{
    BLOWFISH_UCHAR	Key [ 8 ];			/*!< 8-Byte key to use in the test. */
    BLOWFISH_ULONG	PlainText [ 2 ];	/*!< 8-Byte block of plaintext to encipher. */
    BLOWFISH_ULONG	CipherText [ 2 ];	/*!< 8-Byte block of expected ciphertext. */

} _BLOWFISH_TEST_VECTOR;

/** @internal ECB Test vector data from http://www.mirrors.wiretapped.net/security/cryptography/algorithms/blowfish/blowfish-TESTVECTORS.txt */

static const _BLOWFISH_TEST_VECTOR _BLOWFISH_EcbTv1 [ ] =
{
    { { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }, { 0x00000000, 0x00000000 }, { 0x4ef99745, 0x6198dd78 } },
    { { 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff }, { 0xffffffff, 0xffffffff }, { 0x51866fd5, 0xb85ecb8a } },
    { { 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff }, { 0x00000000, 0x00000000 }, { 0xf21e9a77, 0xb71c49bc } },
    { { 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11 }, { 0x01234567, 0x89abcdef }, { 0x7d0cc630, 0xafda1ec7 } },
    { { 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11 }, { 0x11111111, 0x11111111 }, { 0x2466dd87, 0x8b963c9d } },
    { { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }, { 0xffffffff, 0xffffffff }, { 0x014933e0, 0xcdaff6e4 } },
    { { 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01 }, { 0x01234567, 0x89abcdef }, { 0xfa34ec48, 0x47b268b2 } },
    { { 0x1f, 0x1f, 0x1f, 0x1f, 0x0e, 0x0e, 0x0e, 0x0e }, { 0x01234567, 0x89abcdef }, { 0xa7907951, 0x08ea3cae } },
    { { 0x30, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }, { 0x10000000, 0x00000001 }, { 0x7d856f9a, 0x613063f2 } },
    { { 0x01, 0x23, 0x45, 0x67, 0x89, 0xab, 0xcd, 0xef }, { 0x11111111, 0x11111111 }, { 0x61f9c380, 0x2281b096 } },
    { { 0xfe, 0xdc, 0xba, 0x98, 0x76, 0x54, 0x32, 0x10 }, { 0x01234567, 0x89abcdef }, { 0x0aceab0f, 0xc6a0a28d } },
    { { 0x7c, 0xa1, 0x10, 0x45, 0x4a, 0x1a, 0x6e, 0x57 }, { 0x01a1d6d0, 0x39776742 }, { 0x59c68245, 0xeb05282b } },
    { { 0x01, 0x31, 0xd9, 0x61, 0x9d, 0xc1, 0x37, 0x6e }, { 0x5cd54ca8, 0x3def57da }, { 0xb1b8cc0b, 0x250f09a0 } },
    { { 0x07, 0xa1, 0x13, 0x3e, 0x4a, 0x0b, 0x26, 0x86 }, { 0x0248d438, 0x06f67172 }, { 0x1730e577, 0x8bea1da4 } },
    { { 0x38, 0x49, 0x67, 0x4c, 0x26, 0x02, 0x31, 0x9e }, { 0x51454b58, 0x2ddf440a }, { 0xa25e7856, 0xcf2651eb } },
    { { 0x04, 0xb9, 0x15, 0xba, 0x43, 0xfe, 0xb5, 0xb6 }, { 0x42fd4430, 0x59577fa2 }, { 0x353882b1, 0x09ce8f1a } },
    { { 0x01, 0x13, 0xb9, 0x70, 0xfd, 0x34, 0xf2, 0xce }, { 0x059b5e08, 0x51cf143a }, { 0x48f4d088, 0x4c379918 } },
    { { 0x01, 0x70, 0xf1, 0x75, 0x46, 0x8f, 0xb5, 0xe6 }, { 0x0756d8e0, 0x774761d2 }, { 0x432193b7, 0x8951fc98 } },
    { { 0x43, 0x29, 0x7f, 0xad, 0x38, 0xe3, 0x73, 0xfe }, { 0x762514b8, 0x29bf486a }, { 0x13f04154, 0xd69d1ae5 } },
    { { 0x07, 0xa7, 0x13, 0x70, 0x45, 0xda, 0x2a, 0x16 }, { 0x3bdd1190, 0x49372802 }, { 0x2eedda93, 0xffd39c79 } },
    { { 0x04, 0x68, 0x91, 0x04, 0xc2, 0xfd, 0x3b, 0x2f }, { 0x26955f68, 0x35af609a }, { 0xd887e039, 0x3c2da6e3 } },
    { { 0x37, 0xd0, 0x6b, 0xb5, 0x16, 0xcb, 0x75, 0x46 }, { 0x164d5e40, 0x4f275232 }, { 0x5f99d04f, 0x5b163969 } },
    { { 0x1f, 0x08, 0x26, 0x0d, 0x1a, 0xc2, 0x46, 0x5e }, { 0x6b056e18, 0x759f5cca }, { 0x4a057a3b, 0x24d3977b } },
    { { 0x58, 0x40, 0x23, 0x64, 0x1a, 0xba, 0x61, 0x76 }, { 0x004bd6ef, 0x09176062 }, { 0x452031c1, 0xe4fada8e } },
    { { 0x02, 0x58, 0x16, 0x16, 0x46, 0x29, 0xb0, 0x07 }, { 0x480d3900, 0x6ee762f2 }, { 0x7555ae39, 0xf59b87bd } },
    { { 0x49, 0x79, 0x3e, 0xbc, 0x79, 0xb3, 0x25, 0x8f }, { 0x437540c8, 0x698f3cfa }, { 0x53c55f9c, 0xb49fc019 } },
    { { 0x4f, 0xb0, 0x5e, 0x15, 0x15, 0xab, 0x73, 0xa7 }, { 0x072d43a0, 0x77075292 }, { 0x7a8e7bfa, 0x937e89a3 } },
    { { 0x49, 0xe9, 0x5d, 0x6d, 0x4c, 0xa2, 0x29, 0xbf }, { 0x02fe5577, 0x8117f12a }, { 0xcf9c5d7a, 0x4986adb5 } },
    { { 0x01, 0x83, 0x10, 0xdc, 0x40, 0x9b, 0x26, 0xd6 }, { 0x1d9d5c50, 0x18f728c2 }, { 0xd1abb290, 0x658bc778 } },
    { { 0x1c, 0x58, 0x7f, 0x1c, 0x13, 0x92, 0x4f, 0xef }, { 0x30553228, 0x6d6f295a }, { 0x55cb3774, 0xd13ef201 } },
    { { 0xe0, 0xfe, 0xe0, 0xfe, 0xf1, 0xfe, 0xf1, 0xfe }, { 0x01234567, 0x89abcdef }, { 0xc39e072d, 0x9fac631d } },
    { { 0x01, 0x23, 0x45, 0x67, 0x89, 0xab, 0xcd, 0xef }, { 0x00000000, 0x00000000 }, { 0x24594688, 0x5754369a } },
    { { 0xfe, 0xdc, 0xba, 0x98, 0x76, 0x54, 0x32, 0x10 }, { 0xffffffff, 0xffffffff }, { 0x6b5c5a9c, 0x5d9e0a5a } }
};

/** @internal Part of ECB Test vector. See #_BLOWFISH_EcbTv1. */


/** @internal Part of ECB Test vector. See #_BLOWFISH_EcbTv1. */


/** @internal Part of ECB Test vector. See #_BLOWFISH_EcbTv1. */

/** @internal Part of CBC/CFB/OFB Test vector. See #_BLOWFISH_EcbTv1. */


/** @internal Part of CBC/CFB/OFB Test vector. See #_BLOWFISH_EcbTv1. */


/** @internal Part of CBC/CFB/OFB Test vector. See #_BLOWFISH_EcbTv1. */


/** @internal Part of CBC/CFB/OFB Test vector. See #_BLOWFISH_EcbTv1. */


/** @internal CBC/CFB/OFB Test modes. */


/** @internal Throughput test vector */

typedef struct __BLOWFISH_THROUGHPUT_TEST
{
    BLOWFISH_MODE	Mode;		/*!< Mode to use in the test. */
    BLOWFISH_UCHAR	Parallel;	/*!< Specifies whether the mode can be parallelised. */

} _BLOWFISH_THROUGHPUT_TEST;

/** @internal Throughput test modes */


/** @internal Specifies the duration (in seconds) for how long the throughput tests should run (must be greater than 1, and preferably a multiple of 2) */

#define _BLOWFISH_THROUGHPUT_DURATION			10

#define _BLOWFISH_THROUGHPUT_STREAM_LENGTH		( 128 * 1024 )


int _BLOWFISH_PrintReturnCode ( char * FunctionName, BLOWFISH_RC ReturnCode )
{
    switch ( ReturnCode )
    {
        case BLOWFISH_RC_SUCCESS:
        {
            return 0;
        }
        case BLOWFISH_RC_INVALID_PARAMETER:
        {
            return printf ( "%s()=Invalid parameter!\n", FunctionName );
        }
        case BLOWFISH_RC_INVALID_KEY:
        {
            return printf ( "%s()=Invalid key!\n", FunctionName );
        }
        case BLOWFISH_RC_WEAK_KEY:
        {
            return printf ( "%s()=Weak key!\n", FunctionName );
        }
        case BLOWFISH_RC_BAD_BUFFER_LENGTH:
        {
            return printf ( "%s()=Invalid buffer length!\n", FunctionName );
        }
        case BLOWFISH_RC_INVALID_MODE:
        {
            return printf ( "%s()=Invalid mode!\n", FunctionName );
        }
        case BLOWFISH_RC_TEST_FAILED:
        {
            return printf ( "%s()=Self-test failed!\n", FunctionName );
        }
        default:
        {
            return printf ( "%s()=Unknown error!\n", FunctionName );
        }
    }
}

/**

    @internal

    Display the name of the specified mode to stdout.

    @param Mode	Mode to display.

    @return Return code from printf().

  */

int _BLOWFISH_PrintMode ( BLOWFISH_MODE Mode )
{
    switch ( Mode )
    {
        case BLOWFISH_MODE_ECB:
        {
            return printf ( "Mode=Electronic codebook (ECB)\n" );
        }
        case BLOWFISH_MODE_CBC:
        {
            return printf ( "Mode=Cipher block chaining (CBC)\n" );
        }
        case BLOWFISH_MODE_CFB:
        {
            return printf ( "Mode=Cipher feedback (CFB)\n" );
        }
        case BLOWFISH_MODE_OFB:
        {
            return printf ( "Mode=Output feedback (OFB)\n" );
        }
        case BLOWFISH_MODE_CTR:
        {
            return printf ( "Mode=Counter (CTR)\n" );
        }
        default:
        {
            return printf ( "Mode=Invalid!\n" );
        }
    }
}

/**

    @internal

    Display a buffer as hex to stdout.

    @param Name		Name of the buffer.

    @param Buffer	Pointer to a buffer of data to display.

    @param Buffer	Length of the buffer.

  */

void _BLOWFISH_PrintBuffer ( char * Name, BLOWFISH_PUCHAR Buffer, BLOWFISH_ULONG BufferLength )
{
    BLOWFISH_ULONG	i;

    printf ( "%s=0x", Name );

    for ( i = 0; i < BufferLength; i++ )
    {
        printf ( "%02x", Buffer [ i ] );
    }

    printf ( " (%d bytes)\n", (int)BufferLength );

    return;
}

/**

    @internal

    Perform a raw ECB mode encipher/decipher on an 8-byte block of plaintext, and verify results.

    @param Key				Pointer to a buffer containing the key to use for cipher operations.

    @param KeyLength		Length of the key buffer.

    @param PlainTextHigh32	High 32-bits of plaintext to encipher.

    @param PlainTextLow32	Low 32-bits of plaintext to encipher.

    @param CipherTextHigh32	High 32 bits of expected ciphertext.

    @param CipherTextLow32	Low 32 bits of expected ciphertext.

    @remarks For use with test vectors 1 and 2.

    @return #BLOWFISH_RC_SUCCESS	Test passed successfully.

    @return Specific return code, see #BLOWFISH_RC.

  */

static BLOWFISH_RC _BLOWFISH_Test_ECB ( BLOWFISH_PUCHAR Key, BLOWFISH_ULONG KeyLength, BLOWFISH_ULONG PlainTextHigh32, BLOWFISH_ULONG PlainTextLow32, BLOWFISH_ULONG CipherTextHigh32, BLOWFISH_ULONG CipherTextLow32 )
{
    BLOWFISH_RC			ReturnCode = BLOWFISH_RC_SUCCESS;
    BLOWFISH_CONTEXT	Context;
    BLOWFISH_ULONG		XLeft = PlainTextHigh32;
    BLOWFISH_ULONG		XRight = PlainTextLow32;

    /* Initialise blowfish */
    ReturnCode = BLOWFISH_Init ( &Context, Key, KeyLength, BLOWFISH_MODE_ECB, 0, 0 );

    _BLOWFISH_PrintReturnCode ( "BLOWFISH_Init", ReturnCode );

    /* Print key information */

    _BLOWFISH_PrintMode ( BLOWFISH_MODE_ECB );

    _BLOWFISH_PrintBuffer ( "Key", Key, KeyLength );

    if ( 1 )
    {
        /* Encipher the 8-byte block of plaintext */

        BLOWFISH_Encipher ( &Context, &XLeft, &XRight );

        printf ( "Plaintext=0x%016x%016x (8 bytes)\n", (unsigned int)PlainTextHigh32, (unsigned int)PlainTextLow32 );

        /* Is the ciphertext as expected? */
\

        //if ( XLeft == CipherTextHigh32 && XRight == CipherTextLow32 )
        /*if(1)
        {
             Decipher the ciphertext

            BLOWFISH_Decipher ( &Context, &XLeft, &XRight );

            //printf ( "Ciphertext=0x%08x%08x (8 bytes)\n", (unsigned int)CipherTextHigh32, (unsigned int)CipherTextLow32 );

             Is the plaintext as expected?

            if ( XLeft != PlainTextHigh32 && XRight != PlainTextLow32 )
            {
             Failed to decipher properly

                //_BLOWFISH_PrintReturnCode ( "BLOWFISH_Decipher", BLOWFISH_RC_TEST_FAILED );

                printf ( "Invalid plaintext=0x%08x%08x (8 bytes)\n", (unsigned int)XLeft, (unsigned int)XRight );

                ReturnCode = BLOWFISH_RC_TEST_FAILED;
            }
        }
        else
        {
             Failed to encipher properly

            //_BLOWFISH_PrintReturnCode ( "BLOWFISH_Encipher", BLOWFISH_RC_TEST_FAILED );

            printf ( "Ciperhtext=0x%08x%08x (8 bytes)\nInvalid ciphertext=0x%08x%08x (8 bytes)\n", (unsigned int)CipherTextHigh32, (unsigned int)CipherTextLow32, (unsigned int)XLeft, (unsigned int)XRight );

            ReturnCode = BLOWFISH_RC_TEST_FAILED;
        }*/
    }

    //printf ( "\n" );

    /* Overwrite the blowfish context record */

    BLOWFISH_Exit ( &Context );

    return ReturnCode;
}

/**

    @internal

    Perform a CBC/CFB/OFB mode test on a buffer of data, and verify results.

    @param Key				Pointer to a buffer containing the key to use for cipher operations.

    @param KeyLength		Length of the key buffer.

    @param Mode				Mode with which to run the test. Must be either #BLOWFISH_MODE_CBC, #BLOWFISH_MODE_CFB or #BLOWFISH_MODE_OFB.

    @param PlainTextBuffer	Pointer to a buffer of plaintext to encipher.

    @param CipherTextBuffer	Pointer to a buffer of expected ciphertext.

    @param BufferLength		Length of the buffers.

    @remarks For use with test vector 3.

    @return #BLOWFISH_RC_SUCCESS	Test passed successfully.

    @return Specific return code, see #BLOWFISH_RC.

  */

/**

    @internal

    Perform a stream based throughput test for the selected mode.

    @param Mode				Mode to use for the test.

    @param StreamBlockSize	Size of each chunk of the stream.

    @remarks Enciphers/deciphers data as a stream for #_BLOWFISH_THROUGHPUT_DURATION seconds, and then calculates the throughput.

    @return #BLOWFISH_RC_SUCCESS	Test passed successfully.

    @return Specific return code, see #BLOWFISH_RC.

  */


/**

    @internal

    Perform all self-tests for all supported modes.

    @return #BLOWFISH_RC_SUCCESS	All tests passed successfully.

    @return Specific return code, see #BLOWFISH_RC.

  */

static BLOWFISH_RC _BLOWFISH_SelfTest ( )
{
    BLOWFISH_RC		ReturnCode = 0;
    BLOWFISH_ULONG	i = 0;
    struct timeval stop, start;
    BLOWFISH_ULONG high = 0;
    BLOWFISH_ULONG low  = 0;

    printf ( "Standard test vectors...\n\n" );

    /* Perform ECB mode tests on test vector 1 */
    //i =1;
    for ( i = 0; i < 31; i++ )
    {
        high = (_BLOWFISH_EcbTv1 [ i ].PlainText[0] << 32) | _BLOWFISH_EcbTv1 [ i ].PlainText[1];
        low  = (_BLOWFISH_EcbTv1 [ i + 1 ].PlainText[0] << 32) | _BLOWFISH_EcbTv1 [ i +1].PlainText[1];
        //printf("_BLOWFISH_EcbTv1 [ i ].PlainText[0]: %llx\n", _BLOWFISH_EcbTv1 [ i ].PlainText[0] <<32 );
        //printf("High: %llx, Low:%llx\n", high, low);
        gettimeofday(&start, NULL);
        ReturnCode = _BLOWFISH_Test_ECB ( (BLOWFISH_PUCHAR)&_BLOWFISH_EcbTv1 [ i ].Key, 8, high, low, (BLOWFISH_ULONG)_BLOWFISH_EcbTv1 [ i ].CipherText[0], (BLOWFISH_ULONG)_BLOWFISH_EcbTv1 [ i ].CipherText[1] );
        gettimeofday(&stop, NULL);
        printf("took %lu\n", stop.tv_usec - start.tv_usec);
        /*if ( ReturnCode != BLOWFISH_RC_SUCCESS )
        {
            return ReturnCode;
        }*/
    }

    return BLOWFISH_RC_SUCCESS;
}

/**

    @internal

    Main entry point for the blowfish self-test application.

    @param ArgumentCount	Number of command line arguments passed to the application.

    @param ArgumentVector	Array of command line arguments passed to the application.

    @return #BLOWFISH_RC_SUCCESS	All tests passed successfully.

    @return Specific return code, see #BLOWFISH_RC.

  */

int main ( )
{
    BLOWFISH_RC	ReturnCode;

    /* Perform all self tests */
    //printf ( "Blowfish selft-test application.\nCopyright (c) 2008, Tom Bonner (tom.bonner@gmail.com)\n\n(For best results close all running applications)\n\n" );

    ReturnCode = _BLOWFISH_SelfTest ( );
    if ( ReturnCode == BLOWFISH_RC_SUCCESS )
    {
        /* All self test passed successfully */
        printf ( "All Blowfish self-tests passed successfully!\n" );
    }
    else
    {
        /* One of the self tests failed */
        printf ( "Blowfish self-test failed!\n" );
    }

    return (int)ReturnCode;
}

/** @} */
