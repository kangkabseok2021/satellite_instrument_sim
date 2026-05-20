import pytest
from httpx import AsyncClient, ASGITransport
from processor.insight_publisher import app, push_metric, _metrics_window

@pytest.fixture(autouse=True)
def clear_metrics():
    _metrics_window.clear()
    yield

@pytest.mark.asyncio
async def test_health_returns_200():
    async with AsyncClient(transport=ASGITransport(app=app), base_url='http://test') as c:
        r = await c.get('/api/health')
    assert r.status_code == 200
    assert r.json()['status'] == 'ok'

@pytest.mark.asyncio
async def test_metrics_schema():
    push_metric({'co2_ppm': 415.0, 'snr_db': 12.0, 'solar_flag': 0,
                 'altitude_m': 5000, 'filtered': 0.75})
    async with AsyncClient(transport=ASGITransport(app=app), base_url='http://test') as c:
        r = await c.get('/api/metrics')
    assert r.status_code == 200
    data = r.json()
    assert 'co2_ppm' in data
    assert 'snr_db'  in data
    assert 'solar_contamination_fraction' in data

@pytest.mark.asyncio
async def test_frames_endpoint():
    for i in range(7):
        push_metric({'co2_ppm': 415.0 + i, 'snr_db': 12.0, 'solar_flag': 0,
                     'altitude_m': 1000*i, 'filtered': 0.5})
    async with AsyncClient(transport=ASGITransport(app=app), base_url='http://test') as c:
        r = await c.get('/api/frames/5')
    assert r.status_code == 200
    assert len(r.json()) == 5

@pytest.mark.asyncio
async def test_metrics_empty_returns_nulls():
    async with AsyncClient(transport=ASGITransport(app=app), base_url='http://test') as c:
        r = await c.get('/api/metrics')
    assert r.json()['co2_ppm'] is None

@pytest.mark.asyncio
async def test_frames_capped_at_available():
    push_metric({'co2_ppm': 415.0, 'snr_db': 12.0, 'solar_flag': 0,
                 'altitude_m': 5000, 'filtered': 0.75})
    async with AsyncClient(transport=ASGITransport(app=app), base_url='http://test') as c:
        r = await c.get('/api/frames/100')  # only 1 stored
    assert len(r.json()) == 1
