
#include "Globals.h"  // NOTE: MSVC stupidness requires this to be the same across all modules

#include "Villager.h"
#include "../World.h"
#include "../BlockArea.h"
#include "../Blocks/BlockHandler.h"





cVillager::cVillager(eVillagerType VillagerType) :
	super("Villager", mtVillager, "", "", 0.6, 1.8),
	m_Type(VillagerType),
	m_DidFindCrops(false),
	m_ActionCountDown(-1)
{
}





void cVillager::DoTakeDamage(TakeDamageInfo & a_TDI)
{
	super::DoTakeDamage(a_TDI);
	if ((a_TDI.Attacker != NULL) && a_TDI.Attacker->IsPlayer())
	{
		if (m_World->GetTickRandomNumber(5) == 3)
		{
			m_World->BroadcastEntityStatus(*this, ENTITY_STATUS_VILLAGER_ANGRY);
		}
	}
}





void cVillager::Tick(float a_Dt, cChunk & a_Chunk)
{
	super::Tick(a_Dt, a_Chunk);
	if (m_ActionCountDown > -1)
	{
		m_ActionCountDown--;
		if (m_ActionCountDown == 0)
		{
			switch (m_Type)
			{
				case vtFarmer: m_World->SetBlock(m_CropsPos.x, m_CropsPos.y, m_CropsPos.z, E_BLOCK_CROPS, 0);
			}
		}
		return;
	}

	if (m_DidFindCrops && !m_bMovingToDestination)
	{
		if ((GetPosition() - m_CropsPos).Length() < 2)
		{
			BLOCKTYPE CropBlock = m_World->GetBlock(m_CropsPos.x, m_CropsPos.y, m_CropsPos.z);
			if (IsBlockFarmable(CropBlock) && m_World->GetBlockMeta(m_CropsPos.x, m_CropsPos.y, m_CropsPos.z) == 0x7)
			{
				cBlockHandler * Handler = cBlockHandler::GetBlockHandler(CropBlock);
				Handler->DropBlock(m_World, this, m_CropsPos.x, m_CropsPos.y, m_CropsPos.z);
				m_World->SetBlock(m_CropsPos.x, m_CropsPos.y, m_CropsPos.z, E_BLOCK_AIR, 0);
				m_ActionCountDown = 20;
			}
		}
		m_DidFindCrops = false;
	}

	if (m_World->GetTickRandomNumber(50) != 0)
	{
		return;
	}

	switch (m_Type)
	{
		case vtFarmer:
		{
			if (!m_DidFindCrops)
			{
				HandleFarmer();
			}
		}
	}
}




void cVillager::HandleFarmer()
{
	cBlockArea Surrounding;
	Surrounding.Read(m_World,
	(int) GetPosX() - 5,
	(int) GetPosX() + 5,
	(int) GetPosY() - 3,
	(int) GetPosY() + 3,
	(int) GetPosZ() - 5,
	(int) GetPosZ() + 5);

	
	for (int I = 0; I < 5; I++)
	{
		for (int Y = 0; Y < 6; Y++)
		{
			// Pick random coordinates and check for crops.
			int X = m_World->GetTickRandomNumber(11);
			int Z = m_World->GetTickRandomNumber(11);

			if (!IsBlockFarmable(Surrounding.GetRelBlockType(X, Y, Z)))
			{
				continue;
			}
			if (Surrounding.GetRelBlockMeta(X, Y, Z) != 0x7)
			{
				continue;
			}

			m_DidFindCrops = true;
			m_CropsPos = Vector3i((int) GetPosX() + X - 5, (int) GetPosY() + Y - 3, (int) GetPosZ() + Z - 5);
			MoveToPosition(Vector3f((float) (m_CropsPos.x + 0.5), (float) m_CropsPos.y, (float) (m_CropsPos.z + 0.5)));
			return;
		} // for Y loop.
	} // Repeat the procces 5 times.
}





bool cVillager::IsBlockFarmable(BLOCKTYPE a_BlockType)
{
	switch (a_BlockType)
	{
		case E_BLOCK_CROPS:
		case E_BLOCK_POTATOES:
		case E_BLOCK_CARROTS:
		{
			return true;
		}
	}
	return false;
}

