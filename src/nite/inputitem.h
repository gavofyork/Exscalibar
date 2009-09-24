#pragma once

#include <QtGui>
#include <QtXml>

class ProcessorItem;

class InputItem: public QGraphicsItem
{
public:
	InputItem(int _i, ProcessorItem* _p);

	ProcessorItem* processorItem() const;
	inline uint index() const { return m_index; }
	inline QSizeF size() const { return m_size; }

	virtual QRectF boundingRect() const;
	virtual void paint(QPainter* _p, const QStyleOptionGraphicsItem*, QWidget*);
	QPointF tip() const;

	static void fromDom(QDomElement _element, ProcessorItem* _parent)
	{
		new InputItem(_element.attribute("index").toInt(), _parent);
	}

	void saveYourself(QDomElement& _parent, QDomDocument& _doc) const
	{
		QDomElement out = _doc.createElement("input");
		out.setAttribute("index", m_index);
		_parent.appendChild(out);
	}

	void typesConfirmed();

	enum { Type = UserType + 2 };
	virtual int type() const { return Type; }

private:
	uint		m_index;
	QSizeF		m_size;
};
