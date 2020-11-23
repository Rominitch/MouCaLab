/// https://github.com/Rominitch/MouCaLab
/// \author  Rominitch
/// \license No license
#pragma once

namespace RT
{
    template<typename DataTypePosition, typename DataTypeSize, typename VecPosition, typename VecSize>
    class Viewport final
    {
        private:
            VecPosition _offset;
            VecSize     _size;

        public:
            Viewport():
            _offset(0, 0),
            _size(0, 0)
            {}

            Viewport(const DataTypePosition iXPos, const DataTypePosition iYPos, const DataTypeSize iWidth, const DataTypeSize iHeight):
            _offset(iXPos, iYPos),
            _size(iWidth, iHeight)
            {}

            ~Viewport(void) = default;

            bool isNull() const
            {
                return _size == VecSize(0, 0);
            }

            DataTypePosition getX() const
            {
                return _offset.x;
            }

            DataTypePosition getY() const
            {
                return _offset.y;
            }

            DataTypeSize getWidth() const
            {
                return _size.x;
            }

            DataTypeSize getHeight() const
            {
                return _size.y;
            }

            void setSize(const DataTypeSize width, const DataTypeSize height)
            {
                _size = VecSize(width, height);
            }

            void setOffset(const DataTypePosition x, const DataTypePosition y)
            {
                _offset = VecPosition(x, y);
            }

            const VecPosition& getOffset() const
            {
                return _offset;
            }

            const VecSize& getSize() const
            {
                return _size;
            }

            bool isInside(const VecPosition& point) const
            {
                const VecPosition limitMax(_offset.x + _size.x, _offset.y + _size.y);
                return glm::all(glm::lessThanEqual(_offset, point))
                    && glm::all(glm::lessThanEqual(point, limitMax));
            }
    };

    using ViewportInt32 = Viewport<int32_t, uint32_t, RT::Array2i, RT::Array2ui>;
}