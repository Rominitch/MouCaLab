/// https://github.com/Rominitch/MouCaLab
/// \author  Rominitch
/// \license No license
#pragma once

namespace Core
{
    /// OS definition
    #ifdef MOUCA_OS_WINDOWS
        using StringOS = std::wstring;

        using StringStreamOS = std::wstringstream;

        /// Converter system
        /*
        using ConverterWUTF8 = std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>>;
        static ConverterWUTF8 g_converter;
        /*
        //-------------------------------
        // Code from https ://stackoverflow.com/questions/55556200/convert-between-stdu8string-and-stdstring

        /// \brief Converts the range of UTF-8 code units to execution encoding.
        /// \tparam R Type of the input range.
        /// \param[in] input Input range.
        /// \return std::string in the execution encoding.
        /// \throw std::invalid_argument If input sequence is ill-formed.
        /// \note This function depends on the global locale.
        template <std::ranges::input_range R>
        requires std::same_as<std::ranges::range_value_t<R>, char8_t>
            std::string ToECSString(R&& input)
        {
            std::string output;
            char temp_buffer[MB_CUR_MAX];
            std::mbstate_t mbstate{};
            auto i = std::ranges::begin(input);
            auto end = std::ranges::end(input);
            for (; i != end; ++i)
            {
                std::size_t result = std::c8rtomb(temp_buffer, *i, &mbstate);
                if (result == -1)
                {
                    throw std::invalid_argument{ "Ill-formed UTF-8 sequence." };
                }
                output.append(temp_buffer, temp_buffer + result);
            }
            return output;
        }

        /// \brief Converts the input range of code units in execution encoding to
        /// UTF-8.
        /// \tparam R Type of the input range.
        /// \param[in] input Input range.
        /// \return std::u8string containing UTF-8 code units.
        /// \throw std::invalid_argument If input sequence is ill-formed or does not end
        /// at the scalar value boundary.
        /// \note This function depends on the global C locale.
        template <std::ranges::input_range R>
        requires std::same_as<std::ranges::range_value_t<R>, char>
            std::u8string ToUTF8String(R&& input)
        {
            std::u8string output;
            char8_t temp_buffer;
            std::mbstate_t mbstate{};
            std::size_t result;
            auto i = std::ranges::begin(input);
            auto end = std::ranges::end(input);
            while (i != end)
            {
                result = std::mbrtoc8(&temp_buffer, std::to_address(i), 1, &mbstate);
                switch (result)
                {
                case 0:
                {
                    ++i;
                    break;
                }
                case std::size_t(-3):
                {
                    break;
                }
                case std::size_t(-2):
                {
                    ++i;
                    break;
                }
                case std::size_t(-1):
                {
                    throw std::invalid_argument{ "Invalid input sequence." };
                }
                default:
                {
                    std::ranges::advance(i, result);
                    break;
                }
                }
                if (result != std::size_t(-2))
                {
                    output.append(1, temp_buffer);
                }
            }
            if (result == -2)
            {
                throw std::invalid_argument{
                    "Code unit sequence does not end at the scalar value "
                    "boundary." };
            }
            return output;
        }

        /// \brief Converts the contiguous range of code units in execution encoding to
        /// UTF-8.
        /// \tparam R Type of the contiguous range.
        /// \param[in] input Input range.
        /// \return std::u8string containing UTF-8 code units.
        /// \throw std::invalid_argument If input sequence is ill-formed or does not end
        /// at the scalar value boundary.
        /// \note This function depends on the global C locale.
        template <std::ranges::contiguous_range R>
        requires std::same_as<std::ranges::range_value_t<R>, char>
            std::u8string ToUTF8String(R&& input)
        {
            std::u8string output;
            char8_t temp_buffer;
            std::mbstate_t mbstate{};
            std::size_t offset = 0;
            std::size_t size = std::ranges::size(input);
            while (offset != size)
            {
                std::size_t result = std::mbrtoc8(&temp_buffer,
                    std::ranges::data(input) + offset, size - offset, &mbstate);
                switch (result)
                {
                case 0:
                {
                    ++offset;
                    break;
                }
                case std::size_t(-3):
                {
                    break;
                }
                case std::size_t(-2):
                {
                    throw std::invalid_argument{
                        "Input sequence does not end at the scalar value "
                        "boundary." };
                }
                case std::size_t(-1):
                {
                    throw std::invalid_argument{ "Invalid input sequence." };
                }
                default:
                {
                    offset += result;
                    break;
                }
                }
                output.append(1, temp_buffer);
            }
            return output;
        }
        */
        //-------------------------------
    #else
    #   error Not implemented String
    #endif

    /// Basic String
    using StringUTF8 = std::u8string;

    using StringCPP     = std::string;
    using StringViewCPP = std::string_view;

    // Standard Program string
    using String     = StringCPP;
    using StringView = StringViewCPP;

    // Custom string
    using StringHex = std::string;
    
    using StringStream = std::stringstream;

    using Path = std::filesystem::path;

    //------------------------------------------------------------------------
    /// \brief  Convert UTF8 string to OS string.
    /// 
    /// \param[in] data: string in UTF8.
    /// \returns OS string
    inline StringOS convertToOS(const StringCPP& data)
    {
        CA2W ca2w(data.c_str());
        return StringOS(ca2w);
        //std::wstring_convert<std::codecvt_utf8<wchar_t>> myconv;
        //return myconv.from_bytes(data);

        //return StringOS(g_converter.from_bytes(data).c_str());
        //C++20 wait std::c8rtomb();
    }

    //------------------------------------------------------------------------
    /// \brief  Convert OS string to UTF8 string.
    /// 
    /// \param[in] data: OS string.
    /// \returns UTF8 string
    inline StringCPP convertToU8(const StringOS& data)
    {
        CW2A cw2a(data.c_str());
        return StringCPP(cw2a);

        //std::wstring_convert<std::codecvt_utf8<wchar_t>> myconv;
        //return myconv.to_bytes(data);
        // 
        //return StringCPP();// (data.begin(), data.end());
        //return StringCPP(g_converter.to_bytes(data).c_str());
    }
}