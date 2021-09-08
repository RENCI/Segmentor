
#include "LineEditDelegate.h"

#include <QLineEdit>

#include <iostream>

LineEditDelegate::LineEditDelegate(QObject* parent)	: QStyledItemDelegate(parent) {
}

QWidget* LineEditDelegate::createEditor(QWidget* parent, const QStyleOptionViewItem&, const QModelIndex&) const
{
	QLineEdit* editor = new QLineEdit(parent);
	editor->setFrame(false);

	return editor;
}

void LineEditDelegate::setEditorData(QWidget* editor, const QModelIndex &index) const
{
	QString text = index.model()->data(index, Qt::EditRole).toString();

	QLineEdit* lineEdit = static_cast<QLineEdit*>(editor);
	lineEdit->setText(text);
}

void LineEditDelegate::setModelData(QWidget* editor, QAbstractItemModel* model, const QModelIndex &index) const
{
	QLineEdit* lineEdit = static_cast<QLineEdit*>(editor);

	model->setData(index, lineEdit->text(), Qt::EditRole);
}

void LineEditDelegate::updateEditorGeometry(QWidget* editor, const QStyleOptionViewItem &option, const QModelIndex &) const 
{
	editor->setGeometry(option.rect);
}
