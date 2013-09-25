/*****************************************************************************\

Copyright (c) Intel Corporation (2013).

    INTEL MAKES NO WARRANTY OF ANY KIND REGARDING THE CODE.  THIS CODE IS
    LICENSED ON AN "AS IS" BASIS AND INTEL WILL NOT PROVIDE ANY SUPPORT,
    ASSISTANCE, INSTALLATION, TRAINING OR OTHER SERVICES.  INTEL DOES NOT
    PROVIDE ANY UPDATES, ENHANCEMENTS OR EXTENSIONS.  INTEL SPECIFICALLY
    DISCLAIMS ANY WARRANTY OF MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR ANY
    PARTICULAR PURPOSE, OR ANY OTHER WARRANTY.  Intel disclaims all liability,
    including liability for infringement of any proprietary rights, relating to
    use of the code. No license, express or implied, by estoppels or otherwise,
    to any intellectual property rights is granted herein.
\*****************************************************************************/
#ifndef OCLHIGHLIGHTER_H
#define OCLHIGHLIGHTER_H

#include <QSyntaxHighlighter>
#include <QFile>
#include <QStringList>
#include <QTextStream>
#include <QDebug>
#include <QCoreApplication>

namespace Validation
{
namespace GUI
{

/**
 * @brief The OCLHighlighter class
 * @detailed class highlight openCL source code. uses in EditTab class. to details see QSyntaxHighlighter in Qt Documentation
 */
class OCLHighlighter : public QSyntaxHighlighter
{
    Q_OBJECT
public:
    explicit OCLHighlighter(QTextDocument *parent = 0);
protected:
    void highlightBlock(const QString &text);
signals:

public slots:
private:
    QStringList readKeyWords();
    struct HighlightingRule
    {
        QRegExp pattern;
        QTextCharFormat format;
    };
    QVector<HighlightingRule> highlightingRules;

    QRegExp commentStartExpression;
    QRegExp commentEndExpression;

    QTextCharFormat keywordFormat;
    QTextCharFormat classFormat;
    QTextCharFormat singleLineCommentFormat;
    QTextCharFormat multiLineCommentFormat;
    QTextCharFormat quotationFormat;
    QTextCharFormat functionFormat;

};

}
}
#endif // OCLHIGHLIGHTER_H
