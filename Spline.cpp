#include "Spline.hpp"



Spline::Spline()
{
}


Spline::~Spline()
{
}

float Spline::GetTime(glm::vec3 p0, glm::vec3 p1)
{
	float a = powf((p1.x - p0.x), 2.0f) + powf((p1.y - p0.y), 2.0f) + powf((p1.z - p0.z), 2.0f);
	float b = powf(a, 0.5f);
	float c = powf(b, 0.5f);

	return c;
}

void Spline::SetLooped(bool bLooped)
{
	b_looped = bLooped;
}

void Spline::AddLocation(glm::vec3 location)
{
	if (location_timeframes.empty())
	{
		location_timeframes.push_back({ location, 0.001 });
		f_locationSplineTime += 0.001;
		return;
	}

	sLocationTimeframe p2;
	if (!b_looped) p2 = location_timeframes[i_locationIndex + 1 < location_timeframes.size() ? i_locationIndex + 1 : location_timeframes.size() - 1];
	else		   p2 = location_timeframes[(i_locationIndex + 1) % location_timeframes.size()];

	f_locationSplineTime -= location_timeframes[i_locationIndex].time;

	location_timeframes[i_locationIndex].time = GetTime(location_timeframes[i_locationIndex].location, location);

	f_locationSplineTime += location_timeframes[i_locationIndex].time;

	if (i_locationIndex == location_timeframes.size() - 1)
	{
		location_timeframes.push_back({ location, GetTime(location, p2.location) });
	}
	else
	{
		auto it = location_timeframes.begin();
		location_timeframes.insert(it + i_locationIndex + 1, { location, GetTime(location, p2.location) });
	}

	++i_locationIndex;

	f_locationSplineTime += location_timeframes[i_locationIndex].time;
}

void Spline::AddRotation(glm::vec3 euler, float time)
{
	float resetTime = f_locationSplineTime - time;

	if (resetTime < 0) return;

	if (rotation_timeframes.empty())
	{
		rotation_timeframes.push_back({ glm::quat(euler), resetTime });
		return;
	}
	size_t i = 0;
	while (i < rotation_timeframes.size() - 1 && time > rotation_timeframes[i].time) time -= rotation_timeframes[i++].time;
	
	if (i == rotation_timeframes.size() - 1)
	{
		rotation_timeframes.push_back({ glm::quat(euler), resetTime });
	}
	else
	{
		auto it = rotation_timeframes.begin();
		rotation_timeframes.insert(it + i + 1, { glm::quat(euler), rotation_timeframes[i].time - time });
	}

	rotation_timeframes[i].time = time;
}

void Spline::AddLocationWithRotation(glm::vec3 location, glm::vec3 euler)
{
	if (location_timeframes.empty())
	{
		location_timeframes.push_back({ location, 0.001 });
		f_locationSplineTime += 0.001;
		return;
	}

	sLocationTimeframe p2;
	if (!b_looped) p2 = location_timeframes[i_locationIndex + 1 < location_timeframes.size() ? i_locationIndex + 1 : location_timeframes.size() - 1];
	else		   p2 = location_timeframes[(i_locationIndex + 1) % location_timeframes.size()];

	f_locationSplineTime -= location_timeframes[i_locationIndex].time;

	location_timeframes[i_locationIndex].time = GetTime(location_timeframes[i_locationIndex].location, location);

	f_locationSplineTime += location_timeframes[i_locationIndex].time;

	if (i_locationIndex == location_timeframes.size() - 1)
	{
		location_timeframes.push_back({ location, GetTime(location, p2.location) });
	}
	else
	{
		auto it = location_timeframes.begin();
		location_timeframes.insert(it + i_locationIndex + 1, { location, GetTime(location, p2.location) });
	}

	++i_locationIndex;

	f_locationSplineTime += location_timeframes[i_locationIndex].time;

	float time = 0;
	for (size_t i = 0; i < i_locationIndex; i++) time += location_timeframes[i].time;

	AddRotation(euler, time);
}

void Spline::move(float speed)
{
	f_locationInterim += speed / 60 / location_timeframes[i_locationIndex].time;
	f_rotationInterim += speed / 60 / location_timeframes[i_locationIndex].time;
	
	while (f_locationInterim > location_timeframes[i_locationIndex].time)
	{
		f_locationInterim -= location_timeframes[i_locationIndex].time;
		if (i_locationIndex == location_timeframes.size() - 1 && b_looped) i_locationIndex = 0;
		else if (i_locationIndex == location_timeframes.size() - 1) i_locationIndex = location_timeframes.size() - 1;
		else i_locationIndex++;
	}

	while (f_locationInterim < 0)
	{
		f_locationInterim += location_timeframes[i_locationIndex > 0 ? i_locationIndex - 1 : b_looped ? location_timeframes.size() - 1 : 0].time;
		if (i_locationIndex == 0 && b_looped) i_locationIndex = location_timeframes.size() - 1;
		else if (i_locationIndex == 0) i_locationIndex = 0;
		else i_locationIndex--;
	}

	while (f_rotationInterim > rotation_timeframes[i_rotationIndex].time)
	{
		f_rotationInterim -= rotation_timeframes[i_rotationIndex].time;
		if (i_rotationIndex == rotation_timeframes.size() - 1 && b_looped) i_rotationIndex = 0;
		else if (i_rotationIndex == rotation_timeframes.size() - 1) i_rotationIndex = rotation_timeframes.size() - 1;
		else i_rotationIndex++;
	}

	while (f_rotationInterim < 0)
	{
		f_rotationInterim += rotation_timeframes[i_rotationIndex > 0 ? i_rotationIndex - 1 : b_looped ? rotation_timeframes.size() - 1 : 0].time;
		if (i_rotationIndex == 0 && b_looped) i_rotationIndex = rotation_timeframes.size() - 1;
		else if (i_rotationIndex == 0) i_rotationIndex = 0;
		else i_rotationIndex--;
	}
}

glm::vec3 Spline::GetSplinePointLocation()
{
	sLocationTimeframe p0, p1, p2, p3;
	if (!b_looped)
	{
		p1 = location_timeframes[i_locationIndex];
		p2 = location_timeframes[i_locationIndex + 1 < location_timeframes.size() ? i_locationIndex + 1 : location_timeframes.size() - 1];
		p3 = location_timeframes[i_locationIndex + 2 < location_timeframes.size() ? i_locationIndex + 2 : location_timeframes.size() - 1];
		p0 = location_timeframes[i_locationIndex > 0 ? i_locationIndex - 1 : 0];
	}
	else
	{
		p1 = location_timeframes[i_locationIndex];
		p2 = location_timeframes[(i_locationIndex + 1) % location_timeframes.size()];
		p3 = location_timeframes[(i_locationIndex + 2) % location_timeframes.size()];
		p0 = location_timeframes[(i_locationIndex) > 0 ? i_locationIndex - 1 : location_timeframes.size() - 1];
	}

	float t0 = -p0.time;
	float t1 = 0;
	float t2 = p1.time;
	float t3 = p2.time + t2;

	glm::vec3 A1 = (t1 - f_locationInterim) / (t1 - t0)*p0.location + (f_locationInterim - t0) / (t1 - t0)*p1.location;
	glm::vec3 A2 = (t2 - f_locationInterim) / (t2 - t1)*p1.location + (f_locationInterim - t1) / (t2 - t1)*p2.location;
	glm::vec3 A3 = (t3 - f_locationInterim) / (t3 - t2)*p2.location + (f_locationInterim - t2) / (t3 - t2)*p3.location;

	glm::vec3 B1 = (t2 - f_locationInterim) / (t2 - t0)*A1 + (f_locationInterim - t0) / (t2 - t0)*A2;
	glm::vec3 B2 = (t3 - f_locationInterim) / (t3 - t1)*A2 + (f_locationInterim - t1) / (t3 - t1)*A3;

	glm::vec3 C = (t2 - f_locationInterim) / (t2 - t1)*B1 + (f_locationInterim - t1) / (t2 - t1)*B2;

	return C;
}

glm::quat Spline::GetSplinePointRotation()
{
	sRotationTimeframe p0, p1, p2, p3;
	if (!b_looped)
	{
		p1 = rotation_timeframes[i_rotationIndex];
		p2 = rotation_timeframes[i_rotationIndex + 1 < rotation_timeframes.size() ? i_rotationIndex + 1 : rotation_timeframes.size() - 1];
		p3 = rotation_timeframes[i_rotationIndex + 2 < rotation_timeframes.size() ? i_rotationIndex + 2 : rotation_timeframes.size() - 1];
		p0 = rotation_timeframes[i_rotationIndex > 0 ? i_rotationIndex - 1 : 0];
	}
	else
	{
		p1 = rotation_timeframes[i_rotationIndex];
		p2 = rotation_timeframes[(i_rotationIndex + 1) % rotation_timeframes.size()];
		p3 = rotation_timeframes[(i_rotationIndex + 2) % rotation_timeframes.size()];
		p0 = rotation_timeframes[(i_rotationIndex) > 0 ? i_rotationIndex - 1 : rotation_timeframes.size() - 1];
	}

	glm::quat s1 = glm::intermediate(p0.rotation, p1.rotation, p2.rotation);
	glm::quat s2 = glm::intermediate(p1.rotation, p2.rotation, p3.rotation);
	
	return glm::squad(p1.rotation, p2.rotation, s1, s2, f_rotationInterim / p1.time);
}

void Spline::next()
{
	i_locationIndex = i_locationIndex < location_timeframes.size() - 1 ? i_locationIndex + 1 : i_locationIndex;
	i_rotationIndex = i_rotationIndex < rotation_timeframes.size() - 1 ? i_rotationIndex + 1 : i_rotationIndex;
}

void Spline::prev()
{
	i_locationIndex = i_locationIndex < 1 ? 0 : i_locationIndex - 1;
	i_rotationIndex = i_rotationIndex < 1 ? 0 : i_rotationIndex - 1;
}

void Spline::removeLocation()
{
	size_t p0, p1, p2;
	if (!b_looped)
	{
		p1 = i_locationIndex;
		p2 = i_locationIndex + 1 < location_timeframes.size() ? i_locationIndex + 1 : location_timeframes.size() - 1;
		p0 = i_locationIndex > 0 ? i_locationIndex - 1 : 0;
	}
	else
	{
		p1 = i_locationIndex;
		p2 = (i_locationIndex + 1) % location_timeframes.size();
		p0 = (i_locationIndex) > 0 ? i_locationIndex - 1 : location_timeframes.size() - 1;
	}

	f_locationSplineTime -= location_timeframes[p1].time;

	if (p0 != p1) f_locationSplineTime -= location_timeframes[p0].time;

	location_timeframes[p0].time += GetTime(location_timeframes[p0].location, location_timeframes[p2].location);

	f_locationSplineTime += location_timeframes[p0].time;

	auto it = location_timeframes.begin();
	location_timeframes.erase(it + i_locationIndex);

	if (i_locationIndex == 0 && b_looped) i_locationIndex = location_timeframes.size() - 1;
	else if (i_locationIndex == 0) i_locationIndex = 0;
	else i_locationIndex--;
}

void Spline::removeRotation()
{
	size_t p0, p1;
	if (!b_looped)
	{
		p1 = i_rotationIndex;
		p0 = i_rotationIndex > 0 ? i_rotationIndex - 1 : 0;
	}
	else
	{
		p1 = i_rotationIndex;
		p0 = (i_rotationIndex) > 0 ? i_rotationIndex - 1 : rotation_timeframes.size() - 1;
	}

	rotation_timeframes[p0].time += rotation_timeframes[p1].time;

	auto it = rotation_timeframes.begin();
	rotation_timeframes.erase(it + i_rotationIndex);

	if (i_rotationIndex == 0 && b_looped) i_rotationIndex = rotation_timeframes.size() - 1;
	else if (i_rotationIndex == 0) i_rotationIndex = 0;
	else i_rotationIndex--;
}

void Spline::print()
{
	for (size_t i = 0; i < location_timeframes.size(); ++i) std::cout << "path.AddLocation({ " << location_timeframes[i].location.x << ", " << location_timeframes[i].location.y << ", " << location_timeframes[i].location.z << " });" << std::endl;

	std::cout << std::endl;

	float time = 0;
	for (size_t i = 0; i < rotation_timeframes.size(); ++i)
	{
		std::cout << "AddRotation({ " << eulerAngles(rotation_timeframes[i].rotation).x << ", " << eulerAngles(rotation_timeframes[i].rotation).y << ", " << eulerAngles(rotation_timeframes[i].rotation).z << " }," << time << ");" << std::endl;
		time += rotation_timeframes[i].time;
	}
}
