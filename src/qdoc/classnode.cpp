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

#include "classnode.h"

#include "functionnode.h"
#include "propertynode.h"
#include "qdocdatabase.h"
#include "qmltypenode.h"

QT_BEGIN_NAMESPACE

/*!
  \class ClassNode
  \brief The ClassNode represents a C++ class.

  It is also used to represent a C++ struct or union. There are some
  actual uses for structs, but I don't think any unions have been
  documented yet.
 */

/*!
  Adds the base class \a node to this class's list of base
  classes. The base class has the specified \a access. This
  is a resolved base class.
 */
void ClassNode::addResolvedBaseClass(Access access, ClassNode *node)
{
    m_bases.append(RelatedClass(access, node));
    node->m_derived.append(RelatedClass(access, this));
}

/*!
  Adds the derived class \a node to this class's list of derived
  classes. The derived class inherits this class with \a access.
 */
void ClassNode::addDerivedClass(Access access, ClassNode *node)
{
    m_derived.append(RelatedClass(access, node));
}

/*!
  Add an unresolved base class to this class node's list of
  base classes. The unresolved base class will be resolved
  before the generate phase of qdoc. In an unresolved base
  class, the pointer to the base class node is 0.
 */
void ClassNode::addUnresolvedBaseClass(Access access, const QStringList &path,
                                       const QString &signature)
{
    m_bases.append(RelatedClass(access, path, signature));
}

/*!
  Add an unresolved \c using clause to this class node's list
  of \c using clauses. The unresolved \c using clause will be
  resolved before the generate phase of qdoc. In an unresolved
  \c using clause, the pointer to the function node is 0.
 */
void ClassNode::addUnresolvedUsingClause(const QString &signature)
{
    m_usingClauses.append(UsingClause(signature));
}

/*!
  Search the child list to find the property node with the
  specified \a name.
 */
PropertyNode *ClassNode::findPropertyNode(const QString &name)
{
    Node *n = findNonfunctionChild(name, &Node::isProperty);

    if (n)
        return static_cast<PropertyNode *>(n);

    PropertyNode *pn = nullptr;

    const QVector<RelatedClass> &bases = baseClasses();
    if (!bases.isEmpty()) {
        for (int i = 0; i < bases.size(); ++i) {
            ClassNode *cn = bases[i].m_node;
            if (cn) {
                pn = cn->findPropertyNode(name);
                if (pn)
                    break;
            }
        }
    }
    const QVector<RelatedClass> &ignoredBases = ignoredBaseClasses();
    if (!ignoredBases.isEmpty()) {
        for (int i = 0; i < ignoredBases.size(); ++i) {
            ClassNode *cn = ignoredBases[i].m_node;
            if (cn) {
                pn = cn->findPropertyNode(name);
                if (pn)
                    break;
            }
        }
    }

    return pn;
}

/*!
  This function does a recursive search of this class node's
  base classes looking for one that has a QML element. If it
  finds one, it returns the pointer to that QML element. If
  it doesn't find one, it returns null.
 */
QmlTypeNode *ClassNode::findQmlBaseNode()
{
    QmlTypeNode *result = nullptr;
    const QVector<RelatedClass> &bases = baseClasses();

    if (!bases.isEmpty()) {
        for (int i = 0; i < bases.size(); ++i) {
            ClassNode *cn = bases[i].m_node;
            if (cn && cn->qmlElement()) {
                return cn->qmlElement();
            }
        }
        for (int i = 0; i < bases.size(); ++i) {
            ClassNode *cn = bases[i].m_node;
            if (cn) {
                result = cn->findQmlBaseNode();
                if (result != nullptr) {
                    return result;
                }
            }
        }
    }
    return result;
}

/*!
  \a fn is an overriding function in this class or in a class
  derived from this class. Find the node for the function that
  \a fn overrides in this class's children or in one of this
  class's base classes. Return a pointer to the overridden
  function or return 0.

  This should be revised because clang provides the path to the
  overridden function. mws 15/12/2018
 */
FunctionNode *ClassNode::findOverriddenFunction(const FunctionNode *fn)
{
    for (auto &bc : m_bases) {
        ClassNode *cn = bc.m_node;
        if (cn == nullptr) {
            cn = QDocDatabase::qdocDB()->findClassNode(bc.m_path);
            bc.m_node = cn;
        }
        if (cn != nullptr) {
            FunctionNode *result = cn->findFunctionChild(fn);
            if (result != nullptr && !result->isInternal() && !result->isNonvirtual()
                && result->hasDoc())
                return result;
            result = cn->findOverriddenFunction(fn);
            if (result != nullptr && !result->isNonvirtual())
                return result;
        }
    }
    return nullptr;
}

/*!
  \a fn is an overriding function in this class or in a class
  derived from this class. Find the node for the property that
  \a fn overrides in this class's children or in one of this
  class's base classes. Return a pointer to the overridden
  property or return 0.
 */
PropertyNode *ClassNode::findOverriddenProperty(const FunctionNode *fn)
{
    for (auto &baseClass : m_bases) {
        ClassNode *cn = baseClass.m_node;
        if (cn == nullptr) {
            cn = QDocDatabase::qdocDB()->findClassNode(baseClass.m_path);
            baseClass.m_node = cn;
        }
        if (cn != nullptr) {
            const NodeList &children = cn->childNodes();
            for (const auto &child : children) {
                if (child->isProperty()) {
                    auto *pn = static_cast<PropertyNode *>(child);
                    if (pn->name() == fn->name() || pn->hasAccessFunction(fn->name())) {
                        if (pn->hasDoc())
                            return pn;
                    }
                }
            }
            PropertyNode *result = cn->findOverriddenProperty(fn);
            if (result != nullptr)
                return result;
        }
    }
    return nullptr;
}

/*!
  Returns true if the class or struct represented by this class
  node must be documented. If this function returns true, then
  qdoc must find a qdoc comment for this class. If it returns
  false, then the class need not be documented.
 */
bool ClassNode::docMustBeGenerated() const
{
    if (!hasDoc() || isPrivate() || isInternal() || isDontDocument())
        return false;
    if (declLocation().fileName().endsWith(QLatin1String("_p.h")) && !hasDoc())
        return false;

    return true;
}

/*!
  A base class of this class node was private or internal.
  That node's list of \a bases is traversed in this function.
  Each of its public base classes is promoted to be a base
  class of this node for documentation purposes. For each
  private or internal class node in \a bases, this function
  is called recursively with the list of base classes from
  that private or internal class node.
 */
void ClassNode::promotePublicBases(const QVector<RelatedClass> &bases)
{
    if (!bases.isEmpty()) {
        for (int i = bases.size() - 1; i >= 0; --i) {
            ClassNode *bc = bases.at(i).m_node;
            if (bc == nullptr)
                bc = QDocDatabase::qdocDB()->findClassNode(bases.at(i).m_path);
            if (bc != nullptr) {
                if (bc->isPrivate() || bc->isInternal())
                    promotePublicBases(bc->baseClasses());
                else
                    m_bases.append(bases.at(i));
            }
        }
    }
}

/*!
  Remove private and internal bases classes from this class's list
  of base classes. When a base class is removed from the list, add
  its base classes to this class's list of base classes.
 */
void ClassNode::removePrivateAndInternalBases()
{
    int i;
    i = 0;
    QSet<ClassNode *> found;

    // Remove private and duplicate base classes.
    while (i < m_bases.size()) {
        ClassNode *bc = m_bases.at(i).m_node;
        if (bc == nullptr)
            bc = QDocDatabase::qdocDB()->findClassNode(m_bases.at(i).m_path);
        if (bc != nullptr
            && (bc->isPrivate() || bc->isInternal() || bc->isDontDocument()
                || found.contains(bc))) {
            RelatedClass rc = m_bases.at(i);
            m_bases.removeAt(i);
            m_ignoredBases.append(rc);
            promotePublicBases(bc->baseClasses());
        } else {
            ++i;
        }
        found.insert(bc);
    }

    i = 0;
    while (i < m_derived.size()) {
        ClassNode *dc = m_derived.at(i).m_node;
        if (dc != nullptr && (dc->isPrivate() || dc->isInternal() || dc->isDontDocument())) {
            m_derived.removeAt(i);
            const QVector<RelatedClass> &dd = dc->derivedClasses();
            for (int j = dd.size() - 1; j >= 0; --j)
                m_derived.insert(i, dd.at(j));
        } else {
            ++i;
        }
    }
}

/*!
 */
void ClassNode::resolvePropertyOverriddenFromPtrs(PropertyNode *pn)
{
    for (const auto &baseClass : qAsConst(baseClasses())) {
        ClassNode *cn = baseClass.m_node;
        if (cn) {
            Node *n = cn->findNonfunctionChild(pn->name(), &Node::isProperty);
            if (n) {
                auto *baseProperty = static_cast<PropertyNode *>(n);
                cn->resolvePropertyOverriddenFromPtrs(baseProperty);
                pn->setOverriddenFrom(baseProperty);
            } else
                cn->resolvePropertyOverriddenFromPtrs(pn);
        }
    }
}

QT_END_NAMESPACE
