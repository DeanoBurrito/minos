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
        const String* const source;
        uint64_t sourcePos;
        StringBuilder builder;

        FormatFlags ParseFlags();
        uint32_t ParseWidth();
        uint32_t ParsePrecision();
        FormatLength ParseLength();

        FormatSpecifier ParseNext();
        String FormatNext(FormatSpecifier, va_list args);

    public:
        StringFormatter(const String* const input) : source(input), sourcePos(0)
        {}

        void ParseAll(size_t maxLength, va_list args);
        string GetString();
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
            if (TryGetUInt32(widthSubstr, outWidth))
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
                if (TryGetUInt32(precSubstr, precision))
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
        if (specifier.width == UINT32_UPPER_LIMIT)
            specifier.width = va_arg(args, int);
        
        //TODO: all this stuff with the length of arguments seems like it could be simplified with decltype.
        switch (specifier.specifier) 
        {
        case FormatType::NoFormat:
            return "NoFormatData";
        case FormatType::VerbatimSpecifier:
            return "%";

        case FormatType::SingleCharacter:
            //NOTE: char gets promoted to int because of legacy reasons in va_list. This plays nicely with utf8, and casting back to char
            return string(va_arg(args, unsigned int));

        case FormatType::String:
        {
            char* inString = va_arg(args, char*);
            if (specifier.precision == DEFAULT_PRECISION)
                return string(inString);
            else
                return string(inString).SubString(0, specifier.precision);
        }

        case FormatType::SignedInteger:
        {
            switch (specifier.length)
            {
            case FormatLength::Long:
                return IntToString(va_arg(args, long), 10);
            case FormatLength::DoubleLong:
                return IntToString(va_arg(args, long long), 10);
            default:
                return IntToString(va_arg(args, int), 10);
            }
        }

        case FormatType::UnsignedIntegerDecimal:
        {
            switch (specifier.length)
            {
            case FormatLength::Long:
                return UIntToString(va_arg(args, unsigned long), 10);
            case FormatLength::DoubleLong:
                return UIntToString(va_arg(args, unsigned long long), 10);
            default:
                return UIntToString(va_arg(args, unsigned int), 10);
            }
        }

        case FormatType::UnsginedIntegerOctal:
        {
            switch (specifier.length)
            {
            case FormatLength::Long:
                return UIntToString(va_arg(args, unsigned long), 8);
            case FormatLength::DoubleLong:
                return UIntToString(va_arg(args, unsigned long long), 8);
            default:
                return UIntToString(va_arg(args, unsigned int), 8);
            }
        }

        case FormatType::UnsignedIntegerHex:
        {
            switch (specifier.length)
            {
            case FormatLength::Long:
                return UIntToString(va_arg(args, unsigned long), 16);
            case FormatLength::DoubleLong:
                return UIntToString(va_arg(args, unsigned long long), 16);
            default:
                return UIntToString(va_arg(args, unsigned int), 16);
            }
        }

        case FormatType::FloatingPointDecimal:
        {
            double consumedArg = va_arg(args, double);
            return "";
        }

        case FormatType::FloatingPointExponentDecimal:
        {
            double consumedArg = va_arg(args, double);
            return "";
        }

        case FormatType::FloatingPointExponentHex:
        {
            double consumedArg = va_arg(args, double);
            return "";
        }

        case FormatType::FloatingPointShortest:
        {
            double consumedArg = va_arg(args, double);
            return "";
        }

        case FormatType::GetCharactersWritten:
        {
            switch (specifier.length)
            {
            case FormatLength::Default:
            {
                int* charsWrittenInt = va_arg(args, int*);
                *charsWrittenInt = static_cast<int>(builder.Size());
                break;
            }
            case FormatLength::Half:
            {
                short* charsWrittenShort = reinterpret_cast<short*>(va_arg(args, int*));
                *charsWrittenShort = static_cast<short>(builder.Size());
                break;
            }
            case FormatLength::DoubleHalf:
            {
                signed char* charsWrittenChar = reinterpret_cast<signed char*>(va_arg(args, int*));
                *charsWrittenChar = static_cast<signed char>(builder.Size());
                break;
            }
            case FormatLength::Long:
            {
                long* charsWrittenLong = reinterpret_cast<long*>(va_arg(args, long*));
                *charsWrittenLong = static_cast<long>(builder.Size());
                break;
            }
            case FormatLength::DoubleLong:
            {
                long long* charsWrittenShort = reinterpret_cast<long long*>(va_arg(args, long long*));
                *charsWrittenShort = static_cast<int>(builder.Size());
                break;
            }
            }
            return "";
        }

        case FormatType::ImplementationDefined:
        {
            void* consumedArg = va_arg(args, void*);
            return "NoFormatData";
        }
        }

        return "FormattingError";
    }

    void StringFormatter::ParseAll(size_t maxLength, va_list args)
    {
        size_t currentLength = 0;
        size_t segmentStart = 0;
        size_t segmentLength = 0;

        while (sourcePos < source->Size())
        {
            if (source->At(sourcePos) != '%')
            {
                segmentLength++;
                sourcePos++;
                continue; //consume characters until we hit the next format token, or end of string
            }

            //hit a format token, store current text and begin parsing
            if (segmentLength > 0)
                builder.Append(source->SubString(segmentStart, segmentLength));

            FormatSpecifier formatSpec = ParseNext();
            builder.Append(FormatNext(formatSpec, args));

            segmentStart = sourcePos; //next segment starts here
            segmentLength = 0;
        }

        if (segmentLength > 0)
            builder.Append(source->SubString(segmentStart, segmentLength));
    }

    string StringFormatter::GetString()
    {
        return builder.ToString();
    }

    String FormatToString(size_t maxLength, const String* const format, ...)
    {
        StringFormatter formatter(format);

        va_list argsList;
        va_start(argsList, format);

        formatter.ParseAll(maxLength, argsList);

        va_end(argsList);

        return formatter.GetString();
    }
}