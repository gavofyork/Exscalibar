#pragma once

#include <QtGui>
#include <QtXml>

class ProcessorItem;

class InputItem: public QGraphicsItem
{
public:
	InputItem(int _i, ProcessorItem* _p);

	inline ProcessorItem* processorItem() const;

	virtual QRectF boundingRect() const;

	virtual void paint(QPainter* _p, const QStyleOptionGraphicsItem*, QWidget*);

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

	enum { Type = UserType + 2 };
	virtual int type() const { return Type; }

private:
	int			m_index;
	QSizeF		m_size;
};
