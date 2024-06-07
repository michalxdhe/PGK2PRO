#ifndef ANIMDATA_H_INCLUDED
#define ANIMDATA_H_INCLUDED

struct BoneInfo
{
	/*id is index in finalBoneMatrices*/
	int id;

	/*offset matrix transforms vertex from model space to bone space*/
	glm::mat4 offset;

};

#endif // ANIMDATA_H_INCLUDED
