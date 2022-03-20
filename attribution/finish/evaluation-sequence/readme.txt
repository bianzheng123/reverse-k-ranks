查看用柯西不等式来决定哪一个user先算哪一个user后算
idea是: 先得到user和每一个data_item的upper bound和lower bound, 将其映射到0和1的区间, 然后计算queryIP, 映射到01区间
通过norm查看哪个大, 大的就优先算
结果发现，就算用了这个想法之后，结果仍然没有变得更好