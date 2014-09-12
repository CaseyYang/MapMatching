本项目是对新加坡轨迹数据进行一些统计分析和处理工作。
现有代码可以：
1. 得到基于隐马尔科夫模型的地图匹配算法中所需要的sigma和beta值：函数CalculateParametersForViterbiAlgorithm
	sigma值：每个轨迹点到最近的路段距离的MAD（中位数平均偏差，即序列中位数和序列中每个元素的差的中位数）
	beta值：相邻轨迹点距离之差和分别对应的最近路段的路网距离之差的差的MAD
	2*(sigma^2)/beta：Wang Yin等人在GIS CUP 2012中提出的地图匹配算法用到的参数值

2. 分割原始轨迹：函数trajSplit
	先行的分割依据为：如果一条轨迹的相邻两轨迹点的欧氏距离除以时间大于maxSpeed（单位是米/秒），则在此处分割这条轨迹
	分割后得到的新轨迹文件放在和程序同目录下

3. 计算轨迹数据平均采样率：函数CalculateAverageSampleRate
	计算方法：先计算每条轨迹相邻采样点的采样间隔的算术平均值，然后再对所有轨迹的采样率计算算术平均

3. 把原始轨迹转为Json格式文件：函数RawTrajToJson
	Json格式为：data={"rawTrajsId":XXX, "points":[{"x":XXX,"y":XXX,"t":XXX},……]}

4. 从高采样率轨迹数据中抽取采样点，形成给定低采样率的轨迹数据：函数DegradeInputFixedIntervals和函数DegradeAnswerFixedIntervals
	对每条高采样率轨迹数据（如华盛顿州的采样间隔为1s的10条轨迹），从第一个采样点开始，每隔sampleRate（sampleRate是想要得到的采样间隔）取一个采样点，得到新的采样间隔为sampleRate的轨迹。每条采样间隔为1s的原始轨迹能得到sampleRate条新的子轨迹（轨迹0：0，sampleRate，sampleRate*2；轨迹1：1，1+sampleRate,1+sampleRate*2；……）。对原始轨迹对应的真实经过路段序列用类似方法进行处理，得到新轨迹对应的真实经过路段序列。

5. 从高采样率轨迹数据中抽取采样点，形成采样率不高于给定sampleRate的轨迹数据：函数DegradeInputFloatIntervals和函数DegradeAnswerFloatIntervals
    对每条高采样率轨迹数据（如新加坡轨迹集中的一个轨迹文件），首先选取第一个采样点。从第二个采样点起，若当前遍历到的采样点和最后一个已选取采样点的采样时间间隔超过sampleRate，则把当前遍历到的采样点输出到新轨迹文件中。这样从原文件中抽取得到一条采样率不高于给定sampleRate的新轨迹。