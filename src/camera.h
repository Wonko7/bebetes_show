struct camera
{
  float zoom;
  float alpha;
  float beta;
  float x;
  float y;
  float xgoto;
  float ygoto;
  t_agent follow_agent;
  int   reached;
};

typedef struct
{
  int type;
  void * clicked;
  int x;
  int y;
} picked;
/* alexandre... t'es un putain de connard */
