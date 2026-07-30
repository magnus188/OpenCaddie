#include "courses/StoredZipReader.h"

#include <QtEndian>

namespace opencaddie::courses {
namespace {
constexpr quint32 LocalHeader = 0x04034b50;
constexpr qsizetype FixedHeaderSize = 30;
constexpr int MaximumFiles = 128;
constexpr qint64 MaximumUncompressedBytes = 256 * 1024 * 1024;

quint16 u16(const QByteArray& bytes, const qsizetype offset) {
    return qFromLittleEndian<quint16>(
        reinterpret_cast<const uchar*>(bytes.constData() + offset));
}

quint32 u32(const QByteArray& bytes, const qsizetype offset) {
    return qFromLittleEndian<quint32>(
        reinterpret_cast<const uchar*>(bytes.constData() + offset));
}

std::optional<ZipContents> fail(QString* error, const QString& message) {
    if (error) *error = message;
    return std::nullopt;
}
} // namespace

std::optional<ZipContents> StoredZipReader::read(const QByteArray& archive,
                                                QString* error) {
    ZipContents result;
    qsizetype offset = 0;
    while (offset + FixedHeaderSize <= archive.size() &&
           u32(archive, offset) == LocalHeader) {
        if (result.files.size() >= MaximumFiles) {
            return fail(error, QStringLiteral("Course archive contains too many files."));
        }
        const quint16 flags = u16(archive, offset + 6);
        const quint16 method = u16(archive, offset + 8);
        const quint32 compressedSize = u32(archive, offset + 18);
        const quint32 uncompressedSize = u32(archive, offset + 22);
        const quint16 nameLength = u16(archive, offset + 26);
        const quint16 extraLength = u16(archive, offset + 28);
        if (flags != 0 || method != 0 || compressedSize != uncompressedSize) {
            return fail(error,
                        QStringLiteral("Course archive must use deterministic stored ZIP entries."));
        }
        const qsizetype nameStart = offset + FixedHeaderSize;
        const qsizetype contentStart = nameStart + nameLength + extraLength;
        const qsizetype next = contentStart + static_cast<qsizetype>(compressedSize);
        if (nameLength == 0 || next > archive.size()) {
            return fail(error, QStringLiteral("Course archive is truncated."));
        }
        const QString name = QString::fromUtf8(
            archive.constData() + nameStart, static_cast<qsizetype>(nameLength));
        if (!isSafeRelativePath(name) || result.files.contains(name)) {
            return fail(error, QStringLiteral("Course archive contains an unsafe path."));
        }
        result.totalUncompressedBytes += uncompressedSize;
        if (result.totalUncompressedBytes > MaximumUncompressedBytes) {
            return fail(error, QStringLiteral("Course archive is too large."));
        }
        result.files.insert(name, archive.mid(contentStart, compressedSize));
        offset = next;
    }
    if (!result.files.contains(QStringLiteral("manifest.json"))) {
        return fail(error, QStringLiteral("Course archive has no manifest."));
    }
    return result;
}

bool StoredZipReader::isSafeRelativePath(const QString& path) {
    if (path.isEmpty() || path.startsWith('/') || path.startsWith('\\') ||
        path.contains('\\')) {
        return false;
    }
    const auto parts = path.split('/');
    return std::ranges::none_of(parts, [](const QString& part) {
        return part.isEmpty() || part == QStringLiteral(".") ||
               part == QStringLiteral("..");
    });
}

} // namespace opencaddie::courses

