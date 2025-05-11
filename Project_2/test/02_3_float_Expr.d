// float
const float Float_01 = -1.0e2f * 3.f,
            Float_02 = Float_01 * Float_01 +  1.f * 3.14f / 0.5f;
const bool Float_comp =   Float_01 > 0.f
                       || Float_01 >= 0.f
                       || Float_01 < 0.f
                       || Float_01 <= 0.f
                       || Float_01 == 0.f
                       || Float_01 != 0.f;
