#include <ting/debug.hpp>
#include <ting/timer.hpp>

#include "MyMaemoFW/Shader.hpp"
#include "MyMaemoFW/GLWindow.hpp"

#include "ListWidget.hpp"


using namespace ting;
using namespace tride;



ListWidget::ListWidget(unsigned numItemsInRow, unsigned itemHeight){
	this->mw = ModalWidget::New(numItemsInRow, itemHeight);
	this->ShowModal(this->mw, false);
}



void ListWidget::OnResize(){
//	TRACE(<< "ListWidget::OnResize(): invoked" << std::endl)
	ASS(this->mw)->Resize(this->Dim());
}



//override
void ListWidget::ModalWidget::Render(const tride::Matr4f& matrix)const{
//	TRACE(<< "ListWidget::ModalWidget::Render(): invoked" << std::endl)

	//set scissor test
	{
		GLWindow &glw = GLWindow::Inst();
		glEnable(GL_SCISSOR_TEST);
//		TRACE(<< "ListWidget::ModalWidget::Render(): matrix = " << matrix << std::endl)
		Vec2f p0 = matrix * Vec2f(0, 0);
		p0 = (p0 + Vec2f(1, 1)) / 2;
		p0.x *= glw.width();
		p0.y *= glw.height();
//		TRACE(<< "ListWidget::ModalWidget::Render(): p0 = " << p0 << std::endl)
		if(glw.IsPortraitMode()){
			glScissor(
					ASSCOND(p0.x - this->Dim().y, >= 0),
					ASSCOND(p0.y, >= 0),
					ASSCOND(this->Dim().y, >= 0),
					ASSCOND(this->Dim().x, >= 0)
				);
		}else{
			glScissor(
					ASSCOND(p0.x, >= 0),
					ASSCOND(p0.y, >= 0),
					ASSCOND(this->Dim().x, >= 0),
					ASSCOND(this->Dim().y, >= 0)
				);
		}
	}

//	SimpleColoringShader &s = SimpleColoringShader::Inst();
//	s.UseProgram();
//	Matr4f matr(matrix);
//	matr.Scale(this->Dim());
//	s.SetMatrix(matr);
//	s.DisableColorPointer();
//	s.SetColor(Vec3f(1, 1, 1));
//	s.DrawQuad01(GL_LINE_LOOP);

//	TRACE(<< "ListWidget::ModalWidget::Render(): rendering children, num children = " << this->Children().size() << std::endl)

	const Ref<Widget> parent = this->Parent();

	const T_ChildList &children = ASS(parent)->Children();

	if(children.size() == 0)
		return;

	int firstVisibleItem = (this->curPos / ASS(int(this->itemHeight))) * int(this->numItemsPerRow);
	ting::util::ClampBottom(firstVisibleItem, 0);

	ASSERT(firstVisibleItem >= 0)

	if(unsigned(firstVisibleItem) >= children.size())
		return;

	unsigned numItemsToRender = ((unsigned(this->Dim().y) / ASS(this->itemHeight)) + 2) * this->numItemsPerRow;
	ting::util::ClampTop(numItemsToRender, children.size() - firstVisibleItem);

//	TRACE(<< "ListWidget::ModalWidget::Render(): numItemsToRender = " << numItemsToRender << std::endl)

	ASSERT(unsigned(firstVisibleItem) < children.size())
	ASSERT(unsigned(firstVisibleItem) + numItemsToRender <= children.size())

	T_ChildConstIter begin = children.begin() + unsigned(firstVisibleItem);
	T_ChildConstIter end = begin + numItemsToRender;
	unsigned index = 0;
	for(T_ChildConstIter i = begin; i < end; ++i, ++index){
		Matr4f matr(matrix);
		matr.Translate(
				(index % ASS(this->numItemsPerRow)) * this->itemDim.x,
				this->Dim().y - int(((index / ASS(this->numItemsPerRow)) + 1) * this->itemHeight) +
						(firstVisibleItem == 0 ? this->curPos : (this->curPos % int(this->itemHeight)))
			);
//		TRACE(<< "ListWidget::ModalWidget::Render(): itemDim = " << this->itemDim << std::endl)
		matr.Translate(
				(this->itemDim - (*i)->Dim()) / 2
			);
		(*i)->RenderWithChildren(matr);
	}

	//disable scissor test
	glDisable(GL_SCISSOR_TEST);
}



//override
bool ListWidget::ModalWidget::OnMouseClick(const tride::Vec2f& pos, EMouseButton button, bool isDown){
//	TRACE(<< "ListWidget::ModalWidget::OnMouseClick(): invoked" << std::endl)

	if(button == LEFT){
		if(isDown){//button down
			this->isDragging = true;
			this->scrollTimestamp = ting::timer::GetTicks();

			//clear scroll records
			while(!this->scrollRecords.empty()){
				this->scrollRecords.pop();
			}

			if(this->IsUpdating()){
				this->clickPos = -1;
			}else{
				this->clickPos = pos.y;
			}
			this->StopUpdating();
		}else{//button up
			this->isDragging = false;

			if(this->clickPos >= 0 && ting::math::Abs(this->clickPos - pos.y) < 10){
				Vec2f absPos = Vec2f(pos.x, this->curPos + this->Dim().y - pos.y);
				int row = absPos.y / this->itemDim.y;
				ASSERT(row >= 0)
				int col = absPos.x / this->itemDim.x;
				ASSERT(col >= 0)
				ASSERT(col < int(this->numItemsPerRow))
				unsigned itemNum = col + row * this->numItemsPerRow;
//				TRACE(<< "ListWidget::ModalWidget::OnMouseClick(): item click, absPos = " << absPos << std::endl)
//				TRACE(<< "ListWidget::ModalWidget::OnMouseClick(): item click, itemNum = " << itemNum << std::endl)
				if(Ref<ListWidget> p = Ref<Widget>(this->Parent()).StaticCast<ListWidget>()){
					p->OnItemClicked(itemNum);
				}
			}else{
				this->HandleDraggingReleased();
			}
		}
	}
	return true;
}



void ListWidget::OnItemClicked(unsigned itemNum){
	if(itemNum >= this->Children().size())
		return;

	this->itemClicked.Emit(this->Children()[itemNum]);
}



void ListWidget::ModalWidget::HandleDraggingReleased(){
	ting::u32 dt = ting::timer::GetTicks() - this->scrollTimestamp;

	if(dt == 0){
		return;
	}

	int dy = 0;
	while(!this->scrollRecords.empty()){
		dy += this->scrollRecords.front().delta;
		this->scrollRecords.pop();
	}
	this->velocity = dy * 1000 / ASSCOND(int(dt), != 0);

//	TRACE(<< "ListWidget::ModalWidget::HandleDraggingReleased(): dt = " << dt << std::endl)
	if(ting::math::Abs(this->velocity) > 100){
//		TRACE(<< "ListWidget::ModalWidget::HandleDraggingReleased(): this->velocity = " << this->velocity << std::endl)
		this->StartUpdating();
	}
}



//override
bool ListWidget::ModalWidget::OnMouseMove(const tride::Vec2f& oldPos, const tride::Vec2f& newPos, const tride::Vec2f& dpos){
//	TRACE(<< "ListWidget::ModalWidget::OnMouseMove(): invoked" << std::endl)

	if(!this->isDragging){
		return true;
	}

	if(this->clickPos >= 0 && ting::math::Abs(this->clickPos - newPos.y) < 10){
		//do nothing
	}else{
		this->clickPos = -1;
	}

	//update velocity tracking
	{
		const unsigned DNumRecordsInQueue = 5;
		ASSERT(this->scrollRecords.size() <= DNumRecordsInQueue)
		if(this->scrollRecords.size() == DNumRecordsInQueue){
			this->scrollTimestamp = this->scrollRecords.front().timestamp;
			this->scrollRecords.pop();
		}

		this->scrollRecords.push(
				ScrollRec(ting::timer::GetTicks(), dpos.y)
			);
	}

	//update children position
	int topThr = this->CalcTopThreshold();

	this->curPos += dpos.y;
	ting::util::ClampTop(this->curPos, topThr);
	ting::util::ClampBottom(this->curPos, 0);

	GLWindow::Inst().SetDirty();
	return true;
}



//override
void ListWidget::ModalWidget::OnMouseOut(const tride::Vec2f& newPos){
	TRACE(<< "ListWidget::ModalWidget::OnMouseOut(): invoked" << std::endl)
	if(!this->isDragging)
		return;

	this->isDragging = false;
	this->HandleDraggingReleased();
}



static const int DSlowDownAcc = 500;//pixels per second^2



//override
bool ListWidget::ModalWidget::Update(ting::u32 dt){
	this->curPos += this->velocity * int(dt) / int(1000);

//	TRACE(<< "ListWidget::ModalWidget::Update(): this->curPos = " << this->curPos << std::endl)

	GLWindow::Inst().SetDirty();

	int topThr = this->CalcTopThreshold();
	if(topThr <= 0){
		this->curPos = 0;
		return true;
	}

	if(this->curPos < 0){
		this->curPos = 0;
		return true;
	}else if(this->curPos > topThr){
		this->curPos = topThr;
		return true;
	}

	int velSign = ting::math::Sign(this->velocity);
	this->velocity -= velSign * DSlowDownAcc * int(dt) / 1000;

	if(ting::math::Sign(this->velocity) != velSign){
		return true;//stop updating
	}

	return false;
}



void ListWidget::ModalWidget::OnResize(){
	this->itemDim.x = this->Dim().x / this->numItemsPerRow;
	this->itemDim.y = this->itemHeight;
}



ListWidget::ModalWidget::ModalWidget(unsigned numItemsInRow, unsigned heightOfTheItem) :
		numItemsPerRow(numItemsInRow),
		itemHeight(heightOfTheItem),
		curPos(0),
		isDragging(false)
{}



