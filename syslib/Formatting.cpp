#include <Formatting.h>
#include <String.h>
#include <StringBuilder.h>
#include <StringExtras.h>
#include <Limits.h>
#include <stdarg.h>

#define DEFAULT_PRECISION 6

namespace sl
{
    enum class FormatFlags : uint8_t
    {
        None,
        LeftJustified,
        ForceSign,
        SpaceInsteadOfSign,
        UseAltImplementation,
        ZerosForPadding,
    };

    enum class FormatType : uint8_t
    {
        NoFormat,
        VerbatimSpecifier,
        
        SingleCharacter,
        String,

        SignedInteger,
        UnsignedIntegerDecimal,
        UnsginedIntegerOctal,
        UnsignedIntegerHex,

        FloatingPointDecimal,
        FloatingPointExponentDecimal,
        FloatingPointExponentHex,
        FloatingPointShortest,

        GetCharactersWritten,
        ImplementationDefined,
    };

    enum class FormatLength : uint8_t
    {
        Default,
        Half,
        DoubleHalf,
        Long,
        DoubleLong,
        //rest are unsupported
    };
    
    struct FormatSpecifier
    {
        FormatFlags flags = FormatFlags::None;
        FormatType specifier = FormatType::NoFormat;
        FormatLength length = FormatLength::Default;
        uint32_t width = 0; //0 unspecified. uint32_max means should consume from format args ('*' spec)
        uint32_t precision : 31; //0 meaning unspecified or default precision.
        bool isBig : 1;
    };
    
    class StringFormatter
    {
    private:
        FormatFlags ParseFlags();
        uint32_t ParseWidth();
        uint32_t ParsePrecision();
        FormatLength ParseLength();

        FormatSpecifier ParseNext();
        String FormatNext(FormatSpecifier, va_list args);

    public:
        const String* const source;
        uint64_t sourcePos;

        StringFormatter(const String* const input) : source(input), sourcePos(0)
        {}

        void ParseAll(size_t maxLength, va_list args);
    };

    FormatFlags StringFormatter::ParseFlags()
    {
        FormatFlags flags = FormatFlags::None;
        char currentChar = source->At(sourcePos);

        switch (currentChar)
        { //TODO: flags are mutually exclusive here, according to spec they shouldnt be.
        case '-':
            flags = FormatFlags::LeftJustified;
            sourcePos++;
            break;
        case '+':
            flags = FormatFlags::ForceSign;
            sourcePos++;
            break;
        case ' ':
            flags = FormatFlags::SpaceInsteadOfSign;
            sourcePos++;
            break;
        case '#':
            flags = FormatFlags::UseAltImplementation;
            sourcePos++;
            break;
        case '0':
            flags = FormatFlags::ZerosForPadding;
            sourcePos++;
            break;

        default:
            break;
        }

        return flags;
    }

    uint32_t StringFormatter::ParseWidth()
    {
        char currentChar = source->At(sourcePos);

        if (currentChar == '*')
        {
            sourcePos++;
            return UINT32_UPPER_LIMIT;
        }
        else if (IsDigit(currentChar))
        {
            size_t start = sourcePos;

            while (IsDigit(currentChar))
                sourcePos++;
            
            string widthSubstr = move(source->SubString(start, sourcePos - start));
            uint32_t outWidth;
            if (widthSubstr.TryGetUInt32(outWidth))
                return outWidth;
        }

        return 0;
    }

    uint32_t StringFormatter::ParsePrecision()
    {
        char currentChar = source->At(sourcePos);
        uint32_t precision = DEFAULT_PRECISION; //setting default, it can overwritten if needed

        if (currentChar == '.')
        {
            sourcePos++;
            size_t start = sourcePos;

            while (IsDigit(currentChar))
                sourcePos++;
            
            if (sourcePos > start)
            {
                string precSubstr = move(source->SubString(start, start - sourcePos));
                uint32_t precision;
                if (precSubstr.TryGetUInt32(precision))
                    precision = precision;
            }
        }
        
        return precision;
    }

    FormatLength StringFormatter::ParseLength()
    {
        char currentChar = source->At(sourcePos);
        switch (currentChar)
        {
        case 'h':
            sourcePos++;
            currentChar = source->At(sourcePos);
            if (currentChar == 'h')
                { 
                    sourcePos++; 
                    return FormatLength::DoubleHalf; 
                }
            else
                return FormatLength::Half;

        case 'l':
            sourcePos++;
            currentChar = source->At(sourcePos);
            if (currentChar == 'l')
            {
                sourcePos++;
                return FormatLength::DoubleLong;
            }
            else
                return FormatLength::Long;

        default:
            return FormatLength::Default;
        }
    }

    FormatSpecifier StringFormatter::ParseNext()
    {
        FormatSpecifier spec; //inits with no format by default, so FormatNext() will handle this gracefully.

        if (source->At(sourcePos) != '%')
            return spec;

        return spec; //TESTING
        
        sourcePos++; //consume leading '%'
        char currentChar = source->At(sourcePos);

        //if its "%%", exit early
        if (currentChar == '%')
        {
            spec.specifier = FormatType::VerbatimSpecifier;
            sourcePos++;
            return spec;
        }
        
        spec.flags = ParseFlags();
        spec.width = ParseWidth();
        spec.precision = ParsePrecision();
        spec.length = ParseLength();

        //pass 5: getting the actual format requested
        currentChar = source->At(sourcePos);
        spec.isBig = false;
        switch (currentChar)
        {
        case 'c':
            sourcePos++;
            spec.specifier = FormatType::SingleCharacter;
            break;
        
        case 's':
            sourcePos++;
            spec.specifier = FormatType::String;
            break;
        
        case 'd':
        case 'i':
            sourcePos++;
            spec.specifier = FormatType::SignedInteger;
            break;
        
        case 'o':
            sourcePos++;
            spec.specifier = FormatType::UnsginedIntegerOctal;
            break;
        
        case 'X':
            spec.isBig = true;
        case 'x':
            sourcePos++;
            spec.specifier = FormatType::UnsignedIntegerHex;
            break;
        
        case 'u':
            sourcePos++;
            spec.specifier = FormatType::UnsignedIntegerDecimal;
            break;
        
        case 'F':
            spec.isBig = true;
        case 'f':
            sourcePos++;
            spec.specifier = FormatType::FloatingPointDecimal;
            break;
        
        case 'E':
            spec.isBig = true;
        case 'e':
            sourcePos++;
            spec.specifier = FormatType::FloatingPointExponentDecimal;
            break;

        case 'A':
            spec.isBig = true;
        case 'a':
            sourcePos++;
            spec.specifier = FormatType::FloatingPointExponentHex;
            break;

        case 'G':
            spec.isBig = true;
        case 'g':
            sourcePos++;
            spec.specifier = FormatType::FloatingPointShortest;
            break;
        
        case 'n':
            sourcePos++;
            spec.specifier = FormatType::GetCharactersWritten;
            break;
        
        case 'p':
            sourcePos++;
            spec.specifier = FormatType::ImplementationDefined;
            break;

        default:
            spec.specifier = FormatType::NoFormat;
        }

        return spec;
    }

    String StringFormatter::FormatNext(FormatSpecifier specifier, va_list args)
    {
        if (specifier.specifier == FormatType::NoFormat)
            return "";
        if (specifier.specifier == FormatType::VerbatimSpecifier)
            return "%";

        
        
        //TODO: emit an error token
        return "";
    }

    void StringFormatter::ParseAll(size_t maxLength, va_list args)
    {

    }
    
    String FormatToString(size_t maxLength, const String* const format, ...)
    {
        StringFormatter formatter(format);
        StringBuilder builder;
        builder.Append("Hello");
        builder.Append(" ");
        builder.Append("world!");
        return builder.ToString();


        va_list argsList;
        va_start(argsList, format);

        formatter.ParseAll(maxLength, argsList);
        
        va_end(argsList);
        
        return builder.ToString();
    }
}