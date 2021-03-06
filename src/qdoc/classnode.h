/****************************************************************************
**
** Copyright (C) 2020 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the tools applications of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:GPL-EXCEPT$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 3 as published by the Free Software
** Foundation with exceptions as appearing in the file LICENSE.GPL3-EXCEPT
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-3.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/

#ifndef CLASSNODE_H
#define CLASSNODE_H

#include "aggregate.h"
#include "relatedclass.h"
#include "usingclause.h"

#include <QtCore/qglobal.h>
#include <QtCore/qstring.h>
#include <QtCore/qvector.h>

QT_BEGIN_NAMESPACE

class FunctionNode;
class PropertyNode;
class QmlTypeNode;

class ClassNode : public Aggregate
{
public:
    ClassNode(NodeType type, Aggregate *parent, const QString &name) : Aggregate(type, parent, name)
    {
    }
    bool isFirstClassAggregate() const override { return true; }
    bool isClassNode() const override { return true; }
    bool isRelatableType() const override { return true; }
    bool isWrapper() const override { return m_wrapper; }
    QString obsoleteLink() const override { return m_obsoleteLink; }
    void setObsoleteLink(const QString &t) override { m_obsoleteLink = t; }
    void setWrapper() override { m_wrapper = true; }

    void addResolvedBaseClass(Access access, ClassNode *node);
    void addDerivedClass(Access access, ClassNode *node);
    void addUnresolvedBaseClass(Access access, const QStringList &path, const QString &signature);
    void addUnresolvedUsingClause(const QString &signature);
    void removePrivateAndInternalBases();
    void resolvePropertyOverriddenFromPtrs(PropertyNode *pn);

    QVector<RelatedClass> &baseClasses() { return m_bases; }
    QVector<RelatedClass> &derivedClasses() { return m_derived; }
    QVector<RelatedClass> &ignoredBaseClasses() { return m_ignoredBases; }
    QVector<UsingClause> &usingClauses() { return m_usingClauses; }

    const QVector<RelatedClass> &baseClasses() const { return m_bases; }
    const QVector<RelatedClass> &derivedClasses() const { return m_derived; }
    const QVector<RelatedClass> &ignoredBaseClasses() const { return m_ignoredBases; }
    const QVector<UsingClause> &usingClauses() const { return m_usingClauses; }

    QmlTypeNode *qmlElement() { return m_qmlElement; }
    void setQmlElement(QmlTypeNode *qcn) { m_qmlElement = qcn; }
    bool isAbstract() const override { return m_abstract; }
    void setAbstract(bool b) override { m_abstract = b; }
    PropertyNode *findPropertyNode(const QString &name);
    QmlTypeNode *findQmlBaseNode();
    FunctionNode *findOverriddenFunction(const FunctionNode *fn);
    PropertyNode *findOverriddenProperty(const FunctionNode *fn);
    bool docMustBeGenerated() const override;

private:
    void promotePublicBases(const QVector<RelatedClass> &bases);

private:
    QVector<RelatedClass> m_bases {};
    QVector<RelatedClass> m_derived {};
    QVector<RelatedClass> m_ignoredBases {};
    QVector<UsingClause> m_usingClauses {};
    bool m_abstract { false };
    bool m_wrapper { false };
    QString m_obsoleteLink {};
    QmlTypeNode *m_qmlElement { nullptr };
};

QT_END_NAMESPACE

#endif // CLASSNODE_H
