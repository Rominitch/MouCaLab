#pragma once

namespace GUI
{
    class Font;

    class Text final
    {
        public:
            enum Alignment
            {
                Left,
                Center,
                Right
            };

            Text(void) = default;
            ~Text(void) = default;

            const Core::String& getLabel() const
            {
                return _label;
            }

            void setLabel(const Core::String& label)
            {
                _label = label;
            }

        private:
            MOUCA_NOCOPY(Text);

            Core::String          _label;
            Alignment           _alignment = Left;
            Core::ColorUC32       _color;
            std::weak_ptr<Font> _font;
    };
}