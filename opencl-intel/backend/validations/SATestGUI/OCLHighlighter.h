// INTEL CONFIDENTIAL
//
// Copyright 2013-2018 Intel Corporation.
//
// This software and the related documents are Intel copyrighted materials, and
// your use of them is governed by the express license under which they were
// provided to you (License). Unless the License provides otherwise, you may not
// use, modify, copy, publish, distribute, disclose or transmit this software or
// the related documents without Intel's prior written permission.
//
// This software and the related documents are provided as is, with no express
// or implied warranties, other than those that are expressly stated in the
// License.

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
