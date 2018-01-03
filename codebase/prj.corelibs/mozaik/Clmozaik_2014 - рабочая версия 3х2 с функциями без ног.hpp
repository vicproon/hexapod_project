/* Header file Clmozaik_2014 */
#ifndef __Clmozaik_2014_HPP__
#define __Clmozaik_2014_HPP__

/* interface */

#include"CtvSt.hpp"
#include"CtvDll.hpp"

EXTERN_C void cdecl UserCalc( VectRPtr _x, VectRPtr _v, VectRPtr _a, integer _isubs, integer _UMMessage, integer &WhatDo );
EXTERN_C void cdecl ControlPanelMessage( VectRPtr _x, VectRPtr _v, VectRPtr _a, integer _isubs, integer _index, double _Value );
EXTERN_C void cdecl TimeFuncCalc( real_ _t, VectRPtr _x, VectRPtr _v, integer _isubs );
EXTERN_C void cdecl Get1stOrderODE( real_ _t, VectRPtr _x, VectRPtr _f, integer _isubs );
EXTERN_C void cdecl Get2ndOrderODE( real_ _t, VectRPtr _x, VectRPtr _v, VectRPtr _f, integer _isubs );
EXTERN_C void cdecl UserConCalc( VectRPtr _x, VectRPtr _v, MatrRPtr _Jacobi, Vec3RPtr _Error, integer _isubs, integer _ic, boolean _predict, integer _nright );
/* implementation */

#include"DGetVars.hpp"
#include"mozaik_2014C.hpp"
#include"_Tmozaik_2014.hpp"

#include <stdio.h>

//��������� ������ ��� �������� ���������� � �������
struct Joint{
    char* name;//��� �������
    int ibody;//������ ���� � �������� ����������� ������
    int isubs;//������ ���������� � �������� ����������� ����
    coordin point;//����� ��������� �������
    coordin orient;//���� ���������� �� ������� ������������ �� ����
    coordin angles;//��������� ����
    coordin angles_index;//������� ��������������� ��������������� ��������� �����
};

//��������� ������ ��� �������� ���������� � ����
struct Leg{
	char* name;//��� ����
	int ibody;//������ ���� � ������� ���������� ������
	int isubs;//������ ����������
	coordin point;//����� ��������� ����
	coordin orient;//���� ���������� �� ���� ������������ �� ��������
	coordin angles;//��������� ����
	coordin angles_index;//������� ��������������� ��������������� ��������� ����� ����
	coordin segments;//����� ���������
};


void getAngles(trans_matr&, coordin&);//������ ��������� ����� �� ������� ��������
int getAxis(int, int, coordin&, coordin&);//��������� ��� �������� � ��� ��
void jointRotate(int, coordin&, double);//��������� ����� ��������� ���� ��� ������� �� �� ��������
void axisRotate(int, int, double);//��������� �������� ����������� ������� �� ��� ��������
void intiJoints(void);//������������� �������� ������
void setVector(double, double, double, coordin&);//������ � ������
int normalizeVector(coordin&, coordin&);
void getTransform(int, trans_matr&);//�������� ������� �������� � �� �������� �����
void getPoint(int, coordin&);//�������� ���������� ��������� ����� ��������

Joint Joints[7];//� ������ 7 ��������
Leg Legs[6];//� ������ 6 ���

coordin Body[4];//���� �� ������� + ������ ��

trans_matr MainBody;//������� �������� � �� �������

coordin LegEnd[6];//���������� ������ ��� � ���������� ��
coordin StepPoint[6];//���������� �������� �����
coordin newStepPoint[6];//����� �������� �����

//������ �������� ��� ���������
int MyVectorIndex[7+7*2+3+6];//+7 ��� ������-�������� �������� +7*2 ��� ���� �������� � ������� +3 ������� ��� �� ������� + 6 ������ ��� //+ 6 �������� ����� + 6 ����� �������� �����

//������ ����� ����� ������� � ����������� ���� ��������������
double BodyAnglesPrev[7];
//������ ����� ����� ������� � �������� ���� ��������������
double BodyAnglesCur[7];

int measured = 0;//����� ������
int init_joints = 0;//���� ������������� �������� �������
int init_legs = 0;//���� ������������� ���

FILE* f = fopen("debug.txt","w");

void getPoint(int i, coordin &v)
{
	GetPoint(Joints[i].ibody,Joints[i].isubs, Joints[i].point, v);
}

void getTransform(int i, trans_matr &A)
{
	GetAi0(Joints[i].ibody,Joints[i].isubs,A);
}

void PrintMatrix(trans_matr &A, char* name)
{
       fprintf(f,"Matrix ");
       fprintf(f,name);
       fprintf(f,":\n");
       fprintf(f,"%lf\t%lf\t%lf\n",A[0][0],A[0][1],A[0][2]);
       fprintf(f,"%lf\t%lf\t%lf\n",A[1][0],A[1][1],A[1][2]);
       fprintf(f,"%lf\t%lf\t%lf\n\n\n",A[2][0],A[2][1],A[2][2]);
}

void PrintVector(coordin &A, char* name)
{
       fprintf(f,"Vector ");
       fprintf(f,name);
       fprintf(f,":\n %lf\t%lf\t%lf\n\n",A[0],A[1],A[2]);
}

void PrintScalar(double A, char* name)
{
       fprintf(f,"Parameter ");
       fprintf(f,name);
       fprintf(f,": %lf\n\n",A);
}

void PrintLine(char* line)
{
       fprintf(f,line);
       fprintf(f,"\n");
}

void PrintJoint(Joint &J, char* name)
{
    fprintf(f,"������ ");
	fprintf(f,name);
	fprintf(f,":\n");
	PrintScalar(J.ibody,"ibody");
	PrintScalar(J.isubs,"isubs");
	PrintVector(J.point,"��������� �����");
	PrintVector(J.orient,"���� ���������� �� �������");
	PrintVector(J.angles,"��������� ����");
	PrintVector(J.angles_index,"������� ��������� ���������������");
	fprintf(f,"\n\n");
}

void PrintLeg(Leg &L, char* name)
{
	fprintf(f,"���� ");
	fprintf(f,name);
	fprintf(f,":\n");
	PrintScalar(L.ibody,"ibody");
	PrintScalar(L.isubs,"isubs");
	PrintVector(L.point,"����� ��������� ����");
	PrintVector(L.orient,"���� ���������� �� ����");
	PrintVector(L.angles,"��������� ���� ����");
	PrintVector(L.angles_index,"������� ��������� ���������������");
	PrintVector(L.segments,"����� ���������");
	fprintf(f,"\n\n");
	
}

void getAngles(trans_matr &A, coordin &a)
{
    a[0] = atan2(-A[0][1],A[1][1]);
    a[1] = atan2(cos(a[0])*A[2][1],A[1][1]);
    a[2] = atan2(-A[2][0],A[2][2]);
}

int getAxis(int J1, int J2, coordin &axis_1, coordin &axis_2)//����� ���� �� ������� GetPoint, �.�. �������� ������� ������ �� ��� ��������. ����� �������� ��� �������� �������������� ��������.
/// � �������� ��� �������� � �� ������� �����. ��� ������ � ������� jointRotate �� ����� ����� ��������� �� ������� �������� ���� ����� � ��� � �� �����
{
    coordin R1, R2;
	///////////////////////////////////////////////////////////////////
	//�������� ��� �����
    GetPoint(Joints[J1].ibody, Joints[J1].isubs, Joints[J1].point, R1);
    GetPoint(Joints[J2].ibody, Joints[J2].isubs, Joints[J2].point, R2);
	///////////////////////////////////////////////////////////////////
	
	for (int i=0;i<3;i++)
	{
		axis_1[i] = R2[i]-R1[i];
		axis_2[i] = axis_1[i];
	}	
	
	
	if((J1 == 1)&&(J2 == 3))//������� ��� � ������ ����������� �����
	{
		trans_matr Rx,Ry,Rz,R;
		coordin ex = {1,0,0};
		coordin ey = {0,1,0};
		coordin ez = {0,0,1};
		coordin buf = {Joints[3].point[0],Joints[3].point[1],0};
		
		//setVector(Joints[3].point[0],Joints[3].point[1],Joints[3].point[2],buf);
		//buf = {Joints[3].point[0],Joints[3].point[1],Joints[3].point[2]}
		//PrintVector(Joints[3].point,"J4");
		
		setVector(buf[0],buf[1]+buf[0],buf[2],buf);
		//PrintVector(buf,"Axis in 1 RF");
		Turning(Joints[2].angles[0],ez,Rz);
		Turning(Joints[2].angles[1],ex,Rx);
		Turning(Joints[2].angles[2],ey,Ry);
		Turning(Joints[2].orient[2],ez,R);
		
		Mult_m_v_3x1(NORMAL,R,buf,buf);
		Mult_m_v_3x1(TRANSPON,Ry,buf,buf);
		Mult_m_v_3x1(TRANSPON,Rx,buf,buf);
		Mult_m_v_3x1(TRANSPON,Rz,buf,buf);
		Mult_m_v_3x1(TRANSPON,R,buf,buf);
		//PrintVector(buf,"Axis in 2 RF");
		
		setVector(buf[0]-Joints[2].point[1],buf[1]+Joints[2].point[1],buf[2],buf);
		Turning(Joints[1].angles[0],ez,Rz);
		Turning(Joints[1].angles[1],ex,Rx);
		Turning(Joints[1].angles[2],ey,Ry);
		Turning(Joints[1].orient[2],ez,R);
		
		Mult_m_v_3x1(NORMAL,R,buf,buf);
		Mult_m_v_3x1(TRANSPON,Ry,buf,buf);
		Mult_m_v_3x1(TRANSPON,Rx,buf,buf);
		Mult_m_v_3x1(TRANSPON,Rz,buf,buf);
		Mult_m_v_3x1(TRANSPON,R,buf,buf);
		//PrintVector(buf,"Axis in 3 RF");
		
		setVector(buf[0],buf[1],buf[2],axis_1);
		//PrintVector(axis,"Calculated axis");
		//PrintLine("##########################################################");
		
		Mult_m_v_3x1(NORMAL,R,buf,buf);
		Mult_m_v_3x1(NORMAL,Rz,buf,buf);
		Mult_m_v_3x1(NORMAL,Rx,buf,buf);
		Mult_m_v_3x1(NORMAL,Ry,buf,buf);
		Mult_m_v_3x1(TRANSPON,R,buf,buf);
		
		Turning(Joints[2].angles[0],ez,Rz);
		Turning(Joints[2].angles[1],ex,Rx);
		Turning(Joints[2].angles[2],ey,Ry);
		Turning(Joints[2].orient[2],ez,R);
		
		Mult_m_v_3x1(NORMAL,R,buf,buf);
		Mult_m_v_3x1(NORMAL,Rz,buf,buf);
		Mult_m_v_3x1(NORMAL,Rx,buf,buf);
		Mult_m_v_3x1(NORMAL,Ry,buf,buf);
		Mult_m_v_3x1(TRANSPON,R,buf,buf);
		
		setVector(buf[0],buf[1],buf[2],axis_2);
	}
	
	if((J1 == 0)&&(J2 == 2))//������� ��� � ������ ������� ����������� �����
	{
		trans_matr Rx,Ry,Rz,R;
		coordin ex = {1,0,0};
		coordin ey = {0,1,0};
		coordin ez = {0,0,1};
		coordin buf = {Joints[2].point[0],Joints[2].point[1],0};
		

		setVector(buf[0]-buf[1],buf[1],buf[2],buf);
		
		Turning(Joints[1].angles[0],ez,Rz);
		Turning(Joints[1].angles[1],ex,Rx);
		Turning(Joints[1].angles[2],ey,Ry);
		Turning(Joints[1].orient[2],ez,R);
		
		Mult_m_v_3x1(NORMAL,R,buf,buf);
		Mult_m_v_3x1(TRANSPON,Ry,buf,buf);
		Mult_m_v_3x1(TRANSPON,Rx,buf,buf);
		Mult_m_v_3x1(TRANSPON,Rz,buf,buf);
		Mult_m_v_3x1(TRANSPON,R,buf,buf);
		
		setVector(buf[0]+Joints[1].point[0],buf[1]+Joints[1].point[0],buf[2],buf);
		Turning(Joints[0].angles[0],ez,Rz);
		Turning(Joints[0].angles[1],ex,Rx);
		Turning(Joints[0].angles[2],ey,Ry);
		Turning(Joints[0].orient[2],ez,R);
		
		Mult_m_v_3x1(NORMAL,R,buf,buf);
		Mult_m_v_3x1(TRANSPON,Ry,buf,buf);
		Mult_m_v_3x1(TRANSPON,Rx,buf,buf);
		Mult_m_v_3x1(TRANSPON,Rz,buf,buf);
		Mult_m_v_3x1(TRANSPON,R,buf,buf);
		
		setVector(buf[0],buf[1],buf[2],axis_1);
		//PrintVector(axis,"Calculated axis 1");
		//PrintLine("##########################################################");
		
		Mult_m_v_3x1(NORMAL,R,buf,buf);
		Mult_m_v_3x1(NORMAL,Rz,buf,buf);
		Mult_m_v_3x1(NORMAL,Rx,buf,buf);
		Mult_m_v_3x1(NORMAL,Ry,buf,buf);
		Mult_m_v_3x1(TRANSPON,R,buf,buf);
		
		Turning(Joints[1].angles[0],ez,Rz);
		Turning(Joints[1].angles[1],ex,Rx);
		Turning(Joints[1].angles[2],ey,Ry);
		Turning(Joints[1].orient[2],ez,R);
		
		Mult_m_v_3x1(NORMAL,R,buf,buf);
		Mult_m_v_3x1(NORMAL,Rz,buf,buf);
		Mult_m_v_3x1(NORMAL,Rx,buf,buf);
		Mult_m_v_3x1(NORMAL,Ry,buf,buf);
		Mult_m_v_3x1(TRANSPON,R,buf,buf);
		
		setVector(buf[0],buf[1],buf[2],axis_2);
	}
	
	if((J1 == 4)&&(J2 == 6))//������� ��� � ������ ������� ����������� �����
	{
		trans_matr Rx,Ry,Rz,R;
		coordin ex = {1,0,0};
		coordin ey = {0,1,0};
		coordin ez = {0,0,1};
		coordin buf = {Joints[6].point[0],Joints[6].point[1],0};
		
		setVector(buf[0]-buf[1],buf[1],buf[2],buf);
		
		Turning(Joints[5].angles[0],ez,Rz);
		Turning(Joints[5].angles[1],ex,Rx);
		Turning(Joints[5].angles[2],ey,Ry);
		Turning(Joints[5].orient[2],ez,R);
		
		Mult_m_v_3x1(NORMAL,R,buf,buf);
		Mult_m_v_3x1(TRANSPON,Ry,buf,buf);
		Mult_m_v_3x1(TRANSPON,Rx,buf,buf);
		Mult_m_v_3x1(TRANSPON,Rz,buf,buf);
		Mult_m_v_3x1(TRANSPON,R,buf,buf);
		
		setVector(buf[0]+Joints[5].point[0],buf[1]+Joints[5].point[0],buf[2],buf);
		Turning(Joints[4].angles[0],ez,Rz);
		Turning(Joints[4].angles[1],ex,Rx);
		Turning(Joints[4].angles[2],ey,Ry);
		Turning(Joints[4].orient[2],ez,R);
		
		Mult_m_v_3x1(NORMAL,R,buf,buf);
		Mult_m_v_3x1(TRANSPON,Ry,buf,buf);
		Mult_m_v_3x1(TRANSPON,Rx,buf,buf);
		Mult_m_v_3x1(TRANSPON,Rz,buf,buf);
		Mult_m_v_3x1(TRANSPON,R,buf,buf);
		
		setVector(buf[0],buf[1],buf[2],axis_1);
		//PrintVector(axis,"Calculated axis 1");
		//PrintLine("##########################################################");
		
		Mult_m_v_3x1(NORMAL,R,buf,buf);
		Mult_m_v_3x1(NORMAL,Rz,buf,buf);
		Mult_m_v_3x1(NORMAL,Rx,buf,buf);
		Mult_m_v_3x1(NORMAL,Ry,buf,buf);
		Mult_m_v_3x1(TRANSPON,R,buf,buf);
		
		Turning(Joints[5].angles[0],ez,Rz);
		Turning(Joints[5].angles[1],ex,Rx);
		Turning(Joints[5].angles[2],ey,Ry);
		Turning(Joints[5].orient[2],ez,R);
		
		Mult_m_v_3x1(NORMAL,R,buf,buf);
		Mult_m_v_3x1(NORMAL,Rz,buf,buf);
		Mult_m_v_3x1(NORMAL,Rx,buf,buf);
		Mult_m_v_3x1(NORMAL,Ry,buf,buf);
		Mult_m_v_3x1(TRANSPON,R,buf,buf);
		
		setVector(buf[0],buf[1],buf[2],axis_2);
	}
	
	if((J1 == 3)&&(J2 == 0))//������� ��� � ������ �������� 1�� ��������
	{
		trans_matr Rx,Ry,Rz,R;
		coordin ex = {1,0,0};
		coordin ey = {0,1,0};
		coordin ez = {0,0,1};
		coordin buf = {Joints[0].point[0],Joints[0].point[1],0};
		
		setVector(-buf[1],buf[1],buf[2],buf);
		setVector(buf[0],buf[1],buf[2],axis_2);
		
		Turning(Joints[3].angles[0],ez,Rz);
		Turning(Joints[3].angles[1],ex,Rx);
		Turning(Joints[3].angles[2],ey,Ry);
		Turning(Joints[3].orient[2],ez,R);
		
		Mult_m_v_3x1(NORMAL,R,buf,buf);
		Mult_m_v_3x1(TRANSPON,Ry,buf,buf);
		Mult_m_v_3x1(TRANSPON,Rx,buf,buf);
		Mult_m_v_3x1(TRANSPON,Rz,buf,buf);
		Mult_m_v_3x1(TRANSPON,R,buf,buf);
		
		setVector(buf[0],buf[1],buf[2],axis_1);
	}
		
	if((J1 == 4)&&(J2 == 5))//������� ��� � ������ �������� 2�� ��������
	{
		trans_matr Rx,Ry,Rz,R;
		coordin ex = {1,0,0};
		coordin ey = {0,1,0};
		coordin ez = {0,0,1};
		coordin buf = {Joints[5].point[0],Joints[5].point[1],0};
		
		setVector(buf[0],buf[0],buf[2],buf);
		setVector(buf[0],buf[1],buf[2],axis_2);
		
		Turning(Joints[4].angles[0],ez,Rz);
		Turning(Joints[4].angles[1],ex,Rx);
		Turning(Joints[4].angles[2],ey,Ry);
		Turning(Joints[4].orient[2],ez,R);
		
		Mult_m_v_3x1(NORMAL,R,buf,buf);
		Mult_m_v_3x1(TRANSPON,Ry,buf,buf);
		Mult_m_v_3x1(TRANSPON,Rx,buf,buf);
		Mult_m_v_3x1(TRANSPON,Rz,buf,buf);
		Mult_m_v_3x1(TRANSPON,R,buf,buf);
		
		setVector(buf[0],buf[1],buf[2],axis_1);
	}
	
	if((J1 == 5)&&(J2 == 6))//������� ��� � ������ �������� 3�� ��������
	{
		trans_matr Rx,Ry,Rz,R;
		coordin ex = {1,0,0};
		coordin ey = {0,1,0};
		coordin ez = {0,0,1};
		coordin buf = {Joints[6].point[0],Joints[6].point[1],0};
		
		setVector(-buf[1],buf[1],buf[2],buf);
		setVector(buf[0],buf[1],buf[2],axis_2);

		Turning(Joints[5].angles[0],ez,Rz);
		Turning(Joints[5].angles[1],ex,Rx);
		Turning(Joints[5].angles[2],ey,Ry);
		Turning(Joints[5].orient[2],ez,R);
		
		Mult_m_v_3x1(NORMAL,R,buf,buf);
		Mult_m_v_3x1(TRANSPON,Ry,buf,buf);
		Mult_m_v_3x1(TRANSPON,Rx,buf,buf);
		Mult_m_v_3x1(TRANSPON,Rz,buf,buf);
		Mult_m_v_3x1(TRANSPON,R,buf,buf);
		
		setVector(buf[0],buf[1],buf[2],axis_1);				
	}
	
	if((J1 == 2)&&(J2 == 3))//������� ��� � ������ �������� 4�� ��������
	{
		trans_matr Rx,Ry,Rz,R;
		coordin ex = {1,0,0};
		coordin ey = {0,1,0};
		coordin ez = {0,0,1};
		coordin buf = {Joints[3].point[0],Joints[3].point[1],0};
		
		setVector(buf[0],buf[0],buf[2],buf);
		setVector(buf[0],buf[1],buf[2],axis_2);
		
		Turning(Joints[2].angles[0],ez,Rz);
		Turning(Joints[2].angles[1],ex,Rx);
		Turning(Joints[2].angles[2],ey,Ry);
		Turning(Joints[2].orient[2],ez,R);
		
		Mult_m_v_3x1(NORMAL,R,buf,buf);
		Mult_m_v_3x1(TRANSPON,Ry,buf,buf);
		Mult_m_v_3x1(TRANSPON,Rx,buf,buf);
		Mult_m_v_3x1(TRANSPON,Rz,buf,buf);
		Mult_m_v_3x1(TRANSPON,R,buf,buf);
		
		setVector(buf[0],buf[1],buf[2],axis_1);
	}
		
	normalizeVector(axis_1,axis_1);
	normalizeVector(axis_2,axis_2);
	return 1;
}

void setVector(double x, double y, double z, coordin &vector)
{
	vector[0] = x;
	vector[1] = y;
	vector[2] = z;
}

void jointRotate(int j, coordin &n, double angle)
{
	//trans_matr A;
	coordin new_n = {n[0],n[1],n[2]};
	//����� ������ ����� ������� GetAi0 ��������� ������� getAxis
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//GetAi0(Joints[j].ibody,Joints[j].isubs,A);//������� �������� � �� �����
	//��� ������������� ��������� ���������� � ������� �� ����������� ���� �� ����� ���������� ���������
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//Mult_m_v_3x1(NORMAL,A,n,new_n);//��� �������� � �� �����
	//setVector(new_n[0],new_n[1],new_n[2],n);
	//////////setVector(n[0],n[1],n[2],new_n);
	//PrintVector(n,"Axis in joint RF");
	
	coordin ex = {1,0,0};
	coordin ey = {0,1,0};
	coordin ez = {0,0,1};
	
	//��������������� �� �������� �����
	trans_matr Rx,Ry,Rz,R,H;
	//PrintLine(Joints[j].name);
	
	Turning(Joints[j].angles[0],ez,Rz);//Turning ���������� ����������������� �������. �������
	//PrintMatrix(Rz,"Rz");
	
	Turning(Joints[j].angles[1],ex,Rx);
	//PrintMatrix(Rx,"Rx");
	
	Turning(Joints[j].angles[2],ey,Ry);
	//PrintMatrix(Ry,"Ry");
	
	Turning(Joints[j].orient[2],ez,R);
	//PrintMatrix(R,"R");
	
	//R_z(-pi/4)*R_z(psi)*R_x(theta)*R_y(phi)*R_z(pi/4)
	Mult_mT_mT(R,Rz,H);
	Mult_m_mT(H,Rx,H);
	Mult_m_mT(H,Ry,H);
	Mult_m_m(H,R,H);
	//PrintMatrix(H,"������� H");

	//������� �������� ������ ��� �� �������� ����
	trans_matr M;
	Turning(angle,new_n,M);
	//PrintMatrix(M,"Turning");
	Mult_mT_m(M,H,H);
	
	Mult_m_m(R,H,H);
	Mult_m_mT(H,R,H);
	
	getAngles(H,Joints[j].angles);
	//PrintVector(Joints[j].angles,"new Angles");
	for(int i = 0;i<3;i++)
	{
		SetIdentifierValue(Joints[j].angles_index[i],Joints[j].isubs,Joints[j].angles[i]);
	}
}

void axisRotate(int J1, int J2, double angle)
{
       coordin axis_1,axis_2;
	   //PrintLine("Rotation Test");
	   //PrintLine(Joints[J1].name);
	   //PrintLine(Joints[J2].name);
	   //PrintScalar(angle,"angle");
	   //PrintLine("###############################################################");
	   getAxis(J1,J2,axis_1,axis_2);
	   //PrintVector(axis,"axis in global RF");
	   jointRotate(J1,axis_1,angle);
	   //PrintVector(axis,"Axis after first rotate");
	   jointRotate(J2,axis_2,-angle);
	   if((J1==1)&&(J2==3))
	   {
			jointRotate(5,axis_1,angle);
	   }
}

int normalizeVector(coordin &v1, coordin &v2)
{
	double module = 0;
	for(int i=0; i<3; i++)
	{
		module += pow(v1[i],2);
	}
	module = pow(module,0.5);
	if(module!=0)
	{
		for(int i=0;i<3;i++)
		{
			v2[i] = v1[i]/module;
		}
		return 1;
	}
	else {return -1;}
}

void updateVectors(double scale)//��������� ��� ������� � �������
{
    coordin zero = {0,0,0};
	
	//��������� ������ ������� ��������
	for(int i = 0;i<7;i++)
	{
		coordin buf;
		getPoint(i,buf);
		buf[0] = buf[0]*scale;
		buf[1] = buf[1]*scale;
		buf[2] = buf[2]*scale;
		SetVectorValue(MyVectorIndex[i+3],buf,zero);
	}
	
	coordin buf;
	coordin axis;
	trans_matr A;
	
	
	//J4J1 in Segment 4
	getPoint(3,buf);
	getAxis(3,0,axis,zero);
	getTransform(3,A);
	Mult_m_v_3x1(TRANSPON,A,axis,axis);
	SetVectorValue(MyVectorIndex[18],axis,buf);
	//J4J1 in Segment 1
	getPoint(0,buf);
	getTransform(0,A);
	Mult_m_v_3x1(TRANSPON,A,zero,zero);
	SetVectorValue(MyVectorIndex[19],zero,buf);
	
	
	//J5J6 in Segment 5
	getPoint(4,buf);
	getAxis(4,5,axis,zero);
	getTransform(4,A);
	Mult_m_v_3x1(TRANSPON,A,axis,axis);
	SetVectorValue(MyVectorIndex[22],axis,buf);
	//J5J6 in Segment 6
	getPoint(5,buf);
	getTransform(5,A);
	Mult_m_v_3x1(TRANSPON,A,zero,zero);
	SetVectorValue(MyVectorIndex[23],zero,buf);
	
	
	//J6J7 in Segment 6
	getPoint(5,buf);
	getAxis(5,6,axis,zero);
	getTransform(5,A);
	Mult_m_v_3x1(TRANSPON,A,axis,axis);
	SetVectorValue(MyVectorIndex[24],axis,buf);
	//J6J7 in Segment 7
	getPoint(6,buf);
	getTransform(6,A);
	Mult_m_v_3x1(TRANSPON,A,zero,zero);
	SetVectorValue(MyVectorIndex[25],zero,buf);
	
	
	//J3J4 in Segment 3
	getPoint(2,buf);
	getAxis(2,3,axis,zero);
	getTransform(2,A);
	Mult_m_v_3x1(TRANSPON,A,axis,axis);
	SetVectorValue(MyVectorIndex[20],axis,buf);
	//J3J4 in Segment 4
	getPoint(3,buf);
	getTransform(3,A);
	Mult_m_v_3x1(TRANSPON,A,zero,zero);
	SetVectorValue(MyVectorIndex[21],zero,buf);
	
	
	//������ ������� ��� J1J3 � ���� �������� ���������
	//J1J3 in Segment 1
	getPoint(0,buf);
	getAxis(0,2,axis,zero);
	getTransform(0,A);
	Mult_m_v_3x1(TRANSPON,A,axis,axis);
	SetVectorValue(MyVectorIndex[10],axis,buf);
	//J1J3 in Segment 3
	getPoint(2,buf);
	getTransform(2,A);
	Mult_m_v_3x1(TRANSPON,A,zero,zero);
	SetVectorValue(MyVectorIndex[11],zero,buf);
	
	
	//������ ������� ��� J2J4 � ���� �������� ���������
	//J2J4 in Segment 2
	getPoint(1,buf);
	getAxis(1,3,axis,zero);
	getTransform(1,A);
	Mult_m_v_3x1(TRANSPON,A,axis,axis);
	SetVectorValue(MyVectorIndex[12],axis,buf);
	//J2J4 in Segment 4
	getPoint(3,buf);
	getTransform(3,A);
	Mult_m_v_3x1(TRANSPON,A,zero,zero);
	SetVectorValue(MyVectorIndex[13], zero,buf);
	
	//������ ������ ��� J6J2 � ���� �������� ���������
	//J6J2 in Segment 6
	getPoint(5,buf);
	getAxis(5,1,axis,zero);
	getTransform(5,A);
	Mult_m_v_3x1(TRANSPON,A,axis,axis);
	SetVectorValue(MyVectorIndex[14],axis,buf);
	//J6J2 in Segment 2
	getPoint(1,buf);
	getTransform(1,A);
	Mult_m_v_3x1(TRANSPON,A,zero,zero);
	SetVectorValue(MyVectorIndex[15],zero,buf);
	
	//������ ������ ��� J5J7
	//J5J7 in Segment 5
	getPoint(4,buf);
	getAxis(4,6,axis,zero);
	getTransform(4,A);
	Mult_m_v_3x1(TRANSPON,A,axis,axis);
	SetVectorValue(MyVectorIndex[16],axis,buf);
	//J5J7 in Segment 7
	getPoint(6,buf);
	getTransform(6,A);
	Mult_m_v_3x1(TRANSPON,A,zero,zero);
	SetVectorValue(MyVectorIndex[17],zero,buf);
	
	
	//Body ex
	GetPoint(Joints[1].ibody, Joints[1].isubs, Joints[1].point,Body[3]);//������ ������� ���������
	getAxis(3,5,Body[0],zero);
	normalizeVector(Body[0],Body[0]);
	SetVectorValue(MyVectorIndex[0],Body[0],Body[3]);
	
	//Body ey
	getAxis(2,0,Body[1],zero);
	normalizeVector(Body[1],Body[1]);
	SetVectorValue(MyVectorIndex[1],Body[1],Body[3]);
	
	//Body ez
	Mult_vect(Body[0],Body[1],Body[2]);
	normalizeVector(Body[2],Body[2]);
	SetVectorValue(MyVectorIndex[2],Body[2],Body[3]);
	
	for(int i=0;i<3;i++)
	{
		for(int j = 0;j<3;j++)
		{
			MainBody[i][j] = Body[j][i];//���������� ������� ���������� ������� �� �������
		}
	}
	//PrintMatrix(MainBody,"MainBody");
}

void updateBodyGeometry(int order, double Longitude, double Lateral_1, double Lateral_2, double Leaf_1, double Leaf_2, double Leaf_3, double Leaf_4)
{
	if(order == 1)
	{
		axisRotate(1,3,Longitude);
		axisRotate(0,2,Lateral_1);
		axisRotate(4,6,Lateral_2);
		axisRotate(3,0,Leaf_1);
		axisRotate(2,3,Leaf_2);
		axisRotate(4,5,Leaf_3);
		axisRotate(5,6,Leaf_4);
	}
	if(order == -1)
	{
		axisRotate(5,6,-Leaf_4);
		axisRotate(4,5,-Leaf_3);
		axisRotate(2,3,-Leaf_2);
		axisRotate(3,0,-Leaf_1);
		axisRotate(4,6,-Lateral_2);
		axisRotate(0,2,-Lateral_1);
		axisRotate(1,3,-Longitude);
	}
}


/* ������� "TimeFuncCalc" ������������ ������������� ��� ������� ������� �������.
�� ����������� ������� ���� "GetPoint" ������ ���� ���������.
���������� ������ ��� � ���� ���������. */
void TimeFuncCalc( real_ _t, VectRPtr _x, VectRPtr _v, integer _isubs )
 {
  _mozaik_2014VarPtr _;
  _ = _PzAll[SubIndx[_isubs-1]-1];
 }

void ForceFuncCalc( real_ _t, VectRPtr _x, VectRPtr _v, integer _isubs )
 {
  _mozaik_2014VarPtr _;
  _ = _PzAll[SubIndx[_isubs-1]-1];
 }

void Get1stOrderODE( real_ _t, VectRPtr _x, VectRPtr _f, integer _isubs )
 {
  _mozaik_2014VarPtr _;
  _ = _PzAll[SubIndx[_isubs-1]-1];
 }

void Get2ndOrderODE( real_ _t, VectRPtr _x, VectRPtr _v, VectRPtr _f, integer _isubs )
 {
  _mozaik_2014VarPtr _;
  _ = _PzAll[SubIndx[_isubs-1]-1];
 }

void UserConCalc( VectRPtr _x, VectRPtr _v, MatrRPtr _Jacobi, Vec3RPtr _Error, integer _isubs, integer _ic, boolean _predict, integer _nright )
 {
  _mozaik_2014VarPtr _;
  _ = _PzAll[SubIndx[_isubs-1]-1];
 }

void UserCalc( VectRPtr _x, VectRPtr _v, VectRPtr _a, integer _isubs, integer _UMMessage, integer &WhatDo )
 {
  integer Key;
  Key = WhatDo;
  WhatDo = NOTHING;

  _mozaik_2014VarPtr _;
  _ = _PzAll[SubIndx[_isubs-1]-1];

  if(init_joints == 0)
{
	init_joints++;
	
	//������ J1
	Joints[0].name = "J1";
	Joints[0].isubs = 1;
	Joints[0].ibody = 1;
	setVector(0, 1.4142135623730950488016887242097*_->a/2, _->c/2, Joints[0].point);
	setVector(0,0,-pi/4, Joints[0].orient);
	setVector(0, 0, 0, Joints[0].angles);
	setVector(6,7,8,Joints[0].angles_index);
	
	//������ J2
	Joints[1].name = "J2";
	Joints[1].isubs = 1;
	Joints[1].ibody = 2;
	setVector(1.4142135623730950488016887242097*_->a/2, 0, _->c/2, Joints[1].point);
	setVector(0,0,-pi/4-pi/2,Joints[1].orient);
	setVector(0,0,0,Joints[1].angles);
	setVector(9,10,11,Joints[1].angles_index);
	
	//������ J3
	Joints[2].name = "J3";
	Joints[2].isubs = 1;
	Joints[2].ibody = 3;
	setVector(0,-1.4142135623730950488016887242097*_->a/2,_->c/2, Joints[2].point);
	setVector(0,0,-pi/4-pi,Joints[2].orient);
	setVector(0,0,0,Joints[2].angles);
	setVector(12,13,14,Joints[2].angles_index);
	
	//������ J4
	Joints[3].name = "J4";
	Joints[3].isubs = 1;
	Joints[3].ibody = 4;
	setVector(-1.4142135623730950488016887242097*_->a/2, 0 ,_->c/2,Joints[3].point);
	setVector(0,0, -pi/4-pi-pi/2 ,Joints[3].orient);
	setVector(0,0,0,Joints[3].angles);
	setVector(15,16,17,Joints[3].angles_index);
	
	//������ J5
	Joints[4].name = "J5";
	Joints[4].isubs = 1;
	Joints[4].ibody = 2;
	setVector(0,1.4142135623730950488016887242097*_->a/2,_->c/2, Joints[4].point);
	setVector(0,0,-pi/4,Joints[4].orient);
	setVector(0,0,0,Joints[4].angles);
	setVector(21,22,23,Joints[4].angles_index);
	
	//������ J6
	Joints[5].name = "J6";
	Joints[5].isubs = 1;
	Joints[5].ibody = 5;
	setVector(1.4142135623730950488016887242097*_->a/2,0,_->c/2, Joints[5].point);
	setVector(0,0,-pi/4-pi/2,Joints[5].orient);//
	setVector(0,0,0,Joints[5].angles);
	setVector(24,25,26,Joints[5].angles_index);
	
	//������ J7
	Joints[6].name = "J7";
	Joints[6].isubs = 1;
	Joints[6].ibody = 6;
	setVector(0,-1.4142135623730950488016887242097*_->a/2,_->c/2,Joints[6].point);
	setVector(0,0,-pi/4-pi,Joints[6].orient);
	setVector(0,0,0,Joints[6].angles);
	setVector(27,28,29,Joints[6].angles_index);
}

	if(init_legs == 0)
	{
		init_legs++;
		
		//Leg1
		Legs[0].name = "Leg1";
		Legs[0].ibody = 1;
		Legs[0].isubs = 1;
		setVector(-1.4142135623730950488016887242097*_->a/2,0,_->c/2,Legs[0].point);
		setVector(0,0,pi,Legs[0].orient);
		setVector(0,0,0,Legs[0].angles);
		setVector(34,35,36,Legs[0].angles_index);
		setVector(_->p1,_->p2,_->p3,Legs[0].segments);
		
		//Leg2
		Legs[1].name = "Leg2";
		Legs[1].ibody = 2;
		Legs[1].isubs = 1;
		setVector(-1.4142135623730950488016887242097*_->a/2,0,_->c/2,Legs[1].point);
		setVector(0,0,pi,Legs[1].orient);
		setVector(0,0,0,Legs[1].angles);
		setVector(37,38,39,Legs[1].angles_index);
		setVector(_->p1,_->p2,_->p3,Legs[1].segments);
		
		//Leg3
		Legs[2].name = "Leg3";
		Legs[2].ibody = 4;
		Legs[2].isubs = 1;
		setVector(-1.4142135623730950488016887242097*_->a/2,0,_->c/2,Legs[2].point);
		setVector(0,0,pi,Legs[2].orient);
		setVector(0,0,0,Legs[2].angles);
		setVector(40,41,42,Legs[2].angles_index);
		setVector(_->p1,_->p2,_->p3,Legs[2].segments);
		
		//Leg4
		Legs[3].name = "Leg4";
		Legs[3].ibody = 4;
		Legs[3].isubs = 1;
		setVector(1.4142135623730950488016887242097*_->a/2,0,_->c/2,Legs[3].point);
		setVector(0,0,0,Legs[3].orient);
		setVector(0,0,0,Legs[3].angles);
		setVector(43,44,45,Legs[3].angles_index);
		setVector(_->p1,_->p2,_->p3,Legs[3].segments);
		
		//Leg5
		Legs[4].name = "Leg5";
		Legs[4].ibody = 3;
		Legs[4].isubs = 1;
		setVector(1.4142135623730950488016887242097*_->a/2,0,_->c/2,Legs[4].point);
		setVector(0,0,0,Legs[4].orient);
		setVector(0,0,0,Legs[4].angles);
		setVector(46,47,48,Legs[4].angles_index);
		setVector(_->p1,_->p2,_->p3,Legs[4].segments);
		
		//Leg6
		Legs[5].name = "Leg6";
		Legs[5].ibody = 6;
		Legs[5].isubs = 1;
		setVector(1.4142135623730950488016887242097*_->a/2,0,_->c/2,Legs[5].point);
		setVector(0,0,0,Legs[5].orient);
		setVector(0,0,0,Legs[5].angles);
		setVector(49,50,51,Legs[5].angles_index);
		setVector(_->p1,_->p2,_->p3,Legs[5].segments);		
	}

  switch( _UMMessage )
   {
    case STEPEND_MESSAGE :
     {

     }
    case FORCESCALC_MESSAGE :
     {
      try
       {

		BodyAnglesCur[0] =  pi/2*sin(t);//���������� ����
		BodyAnglesCur[1] =  pi/2;//*sin(t/2);//������ ���������� ����
		BodyAnglesCur[2] =  pi/2;//*sin(t/3);//������ ���������� ����
		BodyAnglesCur[3] =  0;///-pi/4*sin(t/4);//�������� 1
		BodyAnglesCur[4] =  0;//pi/4*sin(t/5);//�������� 2
		BodyAnglesCur[5] =  0;//-pi/4*sin(t/6);//�������� 3
	    BodyAnglesCur[6] =  0;//pi/4*sin(t/7);//�������� 4
		
		updateBodyGeometry(-1,BodyAnglesPrev[0],BodyAnglesPrev[1],BodyAnglesPrev[2],BodyAnglesPrev[3],BodyAnglesPrev[4],BodyAnglesPrev[5],BodyAnglesPrev[6]);
		updateBodyGeometry(1, BodyAnglesCur[0], BodyAnglesCur[1], BodyAnglesCur[2], BodyAnglesCur[3], BodyAnglesCur[4], BodyAnglesCur[5], BodyAnglesCur[6]);

		BodyAnglesPrev[0] = BodyAnglesCur[0];
		BodyAnglesPrev[1] = BodyAnglesCur[1];
		BodyAnglesPrev[2] = BodyAnglesCur[2];
		BodyAnglesPrev[3] = BodyAnglesCur[3];
		BodyAnglesPrev[4] = BodyAnglesCur[4];
		BodyAnglesPrev[5] = BodyAnglesCur[5];
		BodyAnglesPrev[6] = BodyAnglesCur[6];
		
		updateVectors(5.0);
		
        ForceFuncCalc( t, _x, _v, _isubs );
       }
      catch(...)
       {
        WhatDo = -1;
       }
      break;
     }
    case FIRSTINIT_MESSAGE :
     {
		
		for(int i=0;i<7;i++)
		{
			BodyAnglesPrev[i] = 0;
			BodyAnglesCur[i] = 0;
		}
		
		//��� �� �������
		MyVectorIndex[0] = AddUserVector("Body x axis", vtNoType);
		MyVectorIndex[1] = AddUserVector("Body y axis", vtNoType);
		MyVectorIndex[2] = AddUserVector("Body z axis", vtNoType);
	
	 for(int i = 3;i<10;i++)
	 {
		MyVectorIndex[i] = AddUserVector(Joints[i-3].name,vtNoType);
	 }
		//��� J1J3
		MyVectorIndex[10] = AddUserVector("RotationAxis J1J3 in Segment 1", vtNoType);
		MyVectorIndex[11] = AddUserVector("RotationAxis J1J3 in Segment 3", vtNoType);
		
		//��� J2J4
		MyVectorIndex[12] = AddUserVector("RotationAxis J2J4 in Segment 2", vtNoType);
		MyVectorIndex[13] = AddUserVector("RotationAxis J2J4 in Segment 4", vtNoType);
		
		//��� J6J2
		MyVectorIndex[14] = AddUserVector("RotationAxis J6J2 in Segment 6", vtNoType);
		MyVectorIndex[15] = AddUserVector("RotationAxis J6J2 in Segment 2", vtNoType);
		
		//��� J5J7
		MyVectorIndex[16] = AddUserVector("RotationAxis J5J7 in Segment 5", vtNoType);
		MyVectorIndex[17] = AddUserVector("RotationAxis J5J7 in Segment 7", vtNoType);
		
		//��� J4J1
		MyVectorIndex[18] = AddUserVector("RotationAxis J4J1 in Segment 4", vtNoType);
		MyVectorIndex[19] = AddUserVector("RotationAxis J4J1 in Segment 1", vtNoType);
		
		//��� J3J4
		MyVectorIndex[20] = AddUserVector("RotationAxis J3J4 in Segment 3", vtNoType);
		MyVectorIndex[21] = AddUserVector("RotationAxis J3J4 in Segment 4", vtNoType);
		
		//��� J5J6
		MyVectorIndex[22] = AddUserVector("RotationAxis J5J6 in Segment 5", vtNoType);
		MyVectorIndex[23] = AddUserVector("RotationAxis J5J6 in Segment 6", vtNoType);
		
		//��� J6J7
		MyVectorIndex[24] = AddUserVector("RotationAxis J6J7 in Segment 6", vtNoType);
		MyVectorIndex[25] = AddUserVector("RotationAxis J6J7 in Segment 7", vtNoType);
     }
    case INTEGRFORM_MESSAGE :
     {
		measured = 0;
		init_joints = 0;

		SetIdentifierValue(6,1,0);
		SetIdentifierValue(7,1,0);
		SetIdentifierValue(8,1,0);
		setVector(_->psi2,_->theta2,_->phi2,Joints[0].angles);
		
		SetIdentifierValue(9,1,0);
		SetIdentifierValue(10,1,0);
		SetIdentifierValue(11,1,0);
		setVector(_->psi3,_->theta3,_->phi3,Joints[1].angles);
		
		SetIdentifierValue(12,1,0);
		SetIdentifierValue(13,1,0);
		SetIdentifierValue(14,1,0);
		setVector(_->psi4,_->theta4,_->phi4,Joints[2].angles);
		
		SetIdentifierValue(15,1,0);
		SetIdentifierValue(16,1,0);
		SetIdentifierValue(17,1,0);
		setVector(_->psi1,_->theta1,_->phi1,Joints[3].angles);
		
		SetIdentifierValue(21,1,0);
		SetIdentifierValue(22,1,0);
		SetIdentifierValue(23,1,0);
		setVector(_->psi5,_->theta5,_->phi5,Joints[4].angles);
		
		SetIdentifierValue(24,1,0);
		SetIdentifierValue(25,1,0);
		SetIdentifierValue(26,1,0);
		setVector(_->psi6,_->theta6,_->phi6,Joints[5].angles);
		
		SetIdentifierValue(27,1,0);
		SetIdentifierValue(28,1,0);
		SetIdentifierValue(29,1,0);
		setVector(_->psi7,_->theta7,_->phi7,Joints[6].angles);	
		
		for(int i=0;i<7;i++)
		{
			BodyAnglesPrev[i] = 0;
			BodyAnglesCur[i] = 0;			
		}
     }
   }
 }

void ControlPanelMessage( VectRPtr _x, VectRPtr _v, VectRPtr _a, integer _isubs, integer _index, double _Value )
 {
  _mozaik_2014VarPtr _;
  _ = _PzAll[SubIndx[_isubs-1]-1];
 }

/* end of file */
#endif
