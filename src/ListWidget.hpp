/* 
 * File:   ListWidget.hpp
 * Author: ivan
 *
 * Created on August 7, 2010, 11:21 PM
 */

#pragma once

#include <queue>

#include "MyMaemoFW/Widget.hpp"
#include "MyMaemoFW/Updater.hpp"



class ListWidget : public Widget{

	class ModalWidget : public Widget, public Updateable{
	public:
		ModalWidget(unsigned numItemsInRow, unsigned heightOfTheItem);

		~ModalWidget()throw(){}
		
		static inline ting::Ref<ModalWidget> New(unsigned numItemsInRow, unsigned heightOfTheItem){
			return ting::Ref<ModalWidget>(new ModalWidget(numItemsInRow, heightOfTheItem));
		}

	private:
		//override
		void Render(const tride::Matr4f& matrix)const;

		//override
		bool OnMouseClick(const tride::Vec2f& pos, EMouseButton button, bool isDown);

		//override
		bool OnMouseMove(const tride::Vec2f& oldPos, const tride::Vec2f& newPos, const tride::Vec2f& dpos);

		//override
		void OnMouseOut(const tride::Vec2f& newPos);

		//override
		bool Update(ting::u32 dt);

		//override
		void OnResize();


		unsigned numItemsPerRow;
		unsigned itemHeight;

		int curPos;
		int velocity;

		tride::Vec2f itemDim;

		bool isDragging;

		ting::u32 scrollTimestamp;
		struct ScrollRec{
			ting::u32 timestamp;
			int delta;

			ScrollRec(ting::u32 timestamp, int delta) :
					timestamp(timestamp), delta(delta)
			{}
		};
		std::queue<ScrollRec> scrollRecords;

		inline int CalcTopThreshold()const{
			const ting::Ref<Widget> parent = this->Parent();
			const T_ChildList &children = ASS(parent)->Children();

			return int(this->itemHeight) * int(
					children.size() / ASS(this->numItemsPerRow)
					+ (children.size() % ASS(this->numItemsPerRow) == 0 ? 0 : 1)
				) - int(this->Dim().y);
		}

		void HandleDraggingReleased();

		float clickPos;
	};


	ting::Ref<ModalWidget> mw;
public:

	ListWidget(unsigned numItemsInRow, unsigned itemHeight);
	
	~ListWidget()throw(){}

	static inline ting::Ref<ListWidget> New(unsigned numItemsInRow, unsigned itemHeight){
		return ting::Ref<ListWidget>(new ListWidget(numItemsInRow, itemHeight));
	}

	ting::Signal1<ting::Ref<Widget> > itemClicked;
private:
	void OnItemClicked(unsigned itemNum);

	//override
	void OnResize();

	//override
	void OnMouseIn(const tride::Vec2f& newPos){
		TRACE(<< "ListWidget::OnMouseIn(): invoked" << std::endl)
	}
};

