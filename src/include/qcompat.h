#include <QHash>
#include <QString>
#include <QtGlobal>
#include <functional>

namespace std
{

#if QT_VERSION < QT_VERSION_CHECK(5, 14, 0)
template <>
struct hash<QString>
{
    std::size_t operator()(const QString &s) const noexcept
    {
        return (size_t)qHash(s);
    }
};
#endif

}
